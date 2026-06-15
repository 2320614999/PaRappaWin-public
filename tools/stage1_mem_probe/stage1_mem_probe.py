#!/usr/bin/env python3
"""
Detachable Stage1 memory/debug-server sidecar.

This tool is intentionally external to PaRappaWin. It talks to the existing
DebugServer for frame/status synchronization and optionally reads whitelisted
memory fields from PaRappaWin.exe with ReadProcessMemory.
"""

from __future__ import annotations

import argparse
import csv
import ctypes
import ctypes.wintypes as wt
import json
import os
import re
import struct
import subprocess
import sys
import time
from pathlib import Path
from typing import Any


TOOL_DIR = Path(__file__).resolve().parent
REPO_DIR = TOOL_DIR.parents[1]
sys.path.insert(0, str(REPO_DIR))

from debug_client import DebugClient  # noqa: E402


DEFAULT_PROCESS = "PaRappaWin.exe"
DEFAULT_COMMANDS = [
    "status",
    "json",
    "stage1ovl rail",
    "stage1ovl handoff",
]

INT_HEADER_SIZE = 0x2000
INT_SECTOR_SIZE = 0x800
INT_ENTRY_SIZE = 20
INT_BLOCK_TYPE_MEM = 3

PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ = 0x0010
TH32CS_SNAPMODULE = 0x00000008
TH32CS_SNAPMODULE32 = 0x00000010
INVALID_HANDLE_VALUE = wt.HANDLE(-1).value


class MODULEENTRY32W(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD),
        ("th32ModuleID", wt.DWORD),
        ("th32ProcessID", wt.DWORD),
        ("GlblcntUsage", wt.DWORD),
        ("ProccntUsage", wt.DWORD),
        ("modBaseAddr", ctypes.POINTER(wt.BYTE)),
        ("modBaseSize", wt.DWORD),
        ("hModule", wt.HMODULE),
        ("szModule", wt.WCHAR * 256),
        ("szExePath", wt.WCHAR * 260),
    ]


kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
kernel32.OpenProcess.argtypes = [wt.DWORD, wt.BOOL, wt.DWORD]
kernel32.OpenProcess.restype = wt.HANDLE
kernel32.CloseHandle.argtypes = [wt.HANDLE]
kernel32.CloseHandle.restype = wt.BOOL
kernel32.ReadProcessMemory.argtypes = [
    wt.HANDLE,
    wt.LPCVOID,
    wt.LPVOID,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
kernel32.ReadProcessMemory.restype = wt.BOOL
kernel32.CreateToolhelp32Snapshot.argtypes = [wt.DWORD, wt.DWORD]
kernel32.CreateToolhelp32Snapshot.restype = wt.HANDLE
kernel32.Module32FirstW.argtypes = [wt.HANDLE, ctypes.POINTER(MODULEENTRY32W)]
kernel32.Module32FirstW.restype = wt.BOOL
kernel32.Module32NextW.argtypes = [wt.HANDLE, ctypes.POINTER(MODULEENTRY32W)]
kernel32.Module32NextW.restype = wt.BOOL


def _parse_int(value: Any) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        return int(value, 0)
    raise ValueError(f"not an integer: {value!r}")


def _find_pids_by_image(image_name: str) -> list[int]:
    try:
        out = subprocess.check_output(
            ["tasklist", "/FI", f"IMAGENAME eq {image_name}", "/FO", "CSV", "/NH"],
            stderr=subprocess.DEVNULL,
        )
    except Exception:
        return []
    text = out.decode("utf-8", errors="replace")
    pids: list[int] = []
    for row in csv.reader(text.splitlines()):
        if len(row) < 2:
            continue
        if row[0].strip('"').lower() != image_name.lower():
            continue
        try:
            pids.append(int(row[1].strip('"')))
        except ValueError:
            pass
    return pids


def _open_process(pid: int) -> int:
    handle = kernel32.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, False, pid)
    if not handle:
        err = ctypes.get_last_error()
        raise OSError(err, f"OpenProcess failed for pid {pid}")
    return int(handle)


def _module_bases(pid: int) -> dict[str, int]:
    snap = kernel32.CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        pid,
    )
    if int(snap) == INVALID_HANDLE_VALUE:
        err = ctypes.get_last_error()
        raise OSError(err, f"CreateToolhelp32Snapshot failed for pid {pid}")
    try:
        entry = MODULEENTRY32W()
        entry.dwSize = ctypes.sizeof(MODULEENTRY32W)
        bases: dict[str, int] = {}
        ok = kernel32.Module32FirstW(snap, ctypes.byref(entry))
        while ok:
            name = str(entry.szModule)
            bases[name.lower()] = ctypes.cast(entry.modBaseAddr, ctypes.c_void_p).value or 0
            ok = kernel32.Module32NextW(snap, ctypes.byref(entry))
        return bases
    finally:
        kernel32.CloseHandle(snap)


def _read_bytes(handle: int, address: int, size: int) -> bytes:
    buf = (ctypes.c_ubyte * size)()
    read = ctypes.c_size_t(0)
    ok = kernel32.ReadProcessMemory(
        wt.HANDLE(handle),
        wt.LPCVOID(address),
        ctypes.byref(buf),
        size,
        ctypes.byref(read),
    )
    if not ok or read.value != size:
        err = ctypes.get_last_error()
        raise OSError(err, f"ReadProcessMemory failed at 0x{address:X} size={size}")
    return bytes(buf)


def _read_u8(handle: int, address: int) -> int:
    return _read_bytes(handle, address, 1)[0]


def _read_u32(handle: int, address: int) -> int:
    return struct.unpack("<I", _read_bytes(handle, address, 4))[0]


def _read_u64(handle: int, address: int) -> int:
    return struct.unpack("<Q", _read_bytes(handle, address, 8))[0]


TYPE_FORMATS = {
    "u8": ("<B", 1),
    "i8": ("<b", 1),
    "u16": ("<H", 2),
    "i16": ("<h", 2),
    "u32": ("<I", 4),
    "i32": ("<i", 4),
    "u64": ("<Q", 8),
    "i64": ("<q", 8),
    "f32": ("<f", 4),
    "f64": ("<d", 8),
}


def _decode_value(data: bytes, typ: str) -> Any:
    if typ == "bytes":
        return data.hex()
    fmt_size = TYPE_FORMATS.get(typ)
    if fmt_size is None:
        raise ValueError(f"unsupported type: {typ}")
    fmt, size = fmt_size
    return struct.unpack(fmt, data[:size])[0]


def _field_address(field: dict[str, Any], modules: dict[str, int]) -> int:
    if "address" in field:
        return _parse_int(field["address"])
    module_name = str(field.get("module") or DEFAULT_PROCESS).lower()
    base = modules.get(module_name)
    if base is None:
        raise KeyError(f"module not found: {module_name}")
    return base + _parse_int(field.get("rva", 0))


def _read_field(handle: int, modules: dict[str, int], field: dict[str, Any]) -> Any:
    typ = str(field.get("type") or "u32").lower()
    size = int(field.get("size") or TYPE_FORMATS.get(typ, ("", 0))[1])
    if size <= 0:
        raise ValueError(f"field {field.get('name')} needs size")
    address = _field_address(field, modules)
    for offset in field.get("pointer_chain", []) or []:
        ptr_size = 8 if field.get("ptr64", True) else 4
        ptr_data = _read_bytes(handle, address, ptr_size)
        address = struct.unpack("<Q" if ptr_size == 8 else "<I", ptr_data)[0]
        address += _parse_int(offset)
    return _decode_value(_read_bytes(handle, address, size), typ)


KV_RE = re.compile(r"([A-Za-z0-9_.]+)=(-?0x[0-9A-Fa-f]+|-?\d+)")


def _parse_kv(text: str) -> dict[str, int]:
    out: dict[str, int] = {}
    for key, value in KV_RE.findall(text or ""):
        try:
            out[key] = int(value, 0)
        except ValueError:
            pass
    return out


def _load_profile(path: Path | None) -> dict[str, Any]:
    if path is None:
        return {}
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def _iter_int_blocks(buf: bytes):
    offset = 0
    block_index = 0
    size = len(buf)
    while offset + INT_HEADER_SIZE <= size:
        header = buf[offset : offset + INT_HEADER_SIZE]
        block_type, num_files, sectors = struct.unpack_from("<III", header, 0)
        if block_type == 0xFFFFFFFF:
            return
        entries = []
        max_entries = (INT_HEADER_SIZE - 16) // INT_ENTRY_SIZE
        if num_files > max_entries:
            return
        for i in range(num_files):
            ent_off = 16 + i * INT_ENTRY_SIZE
            entry_size = struct.unpack_from("<I", header, ent_off)[0]
            raw_name = header[ent_off + 4 : ent_off + 20]
            name = raw_name.split(b"\x00", 1)[0].decode("ascii", errors="ignore")
            name = name.rstrip(" \x00") or f"file_{i}"
            entries.append((i, name, entry_size))
        yield block_index, block_type, entries
        offset += INT_HEADER_SIZE + sectors * INT_SECTOR_SIZE
        block_index += 1


def _load_mem_handle_names(compo_path: Path | None) -> dict[int, str]:
    if compo_path is None:
        default_path = REPO_DIR.parent / "S1" / "COMPO01.INT"
        compo_path = default_path if default_path.exists() else None
    if compo_path is None or not compo_path.exists():
        return {}
    blocks = list(_iter_int_blocks(compo_path.read_bytes()))
    for _block_index, block_type, entries in blocks:
        if block_type != INT_BLOCK_TYPE_MEM:
            continue
        return {mem_index + 2: name for mem_index, name, _size in entries}
    return {}


def _sample_stage1_face_tim(
    handle: int | None,
    modules: dict[str, int],
    profile: dict[str, Any],
) -> tuple[dict[str, Any], list[str]]:
    cfg = profile.get("stage1_face_tim") or {}
    if not cfg or not cfg.get("enabled", True):
        return {}, []
    if handle is None:
        return {}, ["stage1_face_tim: process handle unavailable"]

    errors: list[str] = []
    out: dict[str, Any] = {}
    module_name = str(cfg.get("module") or DEFAULT_PROCESS).lower()
    base = modules.get(module_name)
    if base is None:
        return {}, [f"stage1_face_tim: module not found: {module_name}"]

    try:
        mem_map_path = cfg.get("mem_map_compo")
        mem_names = _load_mem_handle_names(Path(mem_map_path) if mem_map_path else None)
    except Exception as exc:
        mem_names = {}
        errors.append(f"stage1_face_tim: mem map load failed: {exc}")

    try:
        runtime_addr = base + _parse_int(cfg["owned_runtime_rva"])
        impl_ptr_offset = _parse_int(cfg.get("impl_ptr_offset", 0))
        impl = _read_u64(handle, runtime_addr + impl_ptr_offset)
        out["runtimeAddress"] = f"0x{runtime_addr:X}"
        out["impl"] = f"0x{impl:X}"
        if impl == 0:
            out["queue"] = {"count": 0, "entries": []}
            return out, errors

        advance_valid_offset = _parse_int(cfg.get("advance_valid_offset", 0x5030))
        last_query_frame_offset = _parse_int(cfg.get("last_query_frame_offset", 0x5034))
        vector_offset = _parse_int(cfg["tim_vector_offset"])
        stride = int(cfg.get("entry_stride", 6))
        max_entries = int(cfg.get("max_entries", 128))

        first = _read_u64(handle, impl + vector_offset)
        last = _read_u64(handle, impl + vector_offset + 8)
        end = _read_u64(handle, impl + vector_offset + 16)
        raw_count_bytes = last - first if last >= first else -1
        count = 0
        entries: list[dict[str, Any]] = []
        if first and last >= first and raw_count_bytes % stride == 0:
            count = raw_count_bytes // stride
            if count > max_entries:
                errors.append(
                    f"stage1_face_tim: queue count {count} exceeds max {max_entries}"
                )
                count = max_entries
            if count:
                data = _read_bytes(handle, first, count * stride)
                for i in range(count):
                    off = i * stride
                    channel = data[off]
                    tim_id = struct.unpack_from("<H", data, off + 2)[0]
                    upload_clut = bool(data[off + 4])
                    entries.append(
                        {
                            "index": i,
                            "channel": channel,
                            "timId": tim_id,
                            "name": mem_names.get(tim_id),
                            "uploadClut": upload_clut,
                        }
                    )
        else:
            errors.append(
                "stage1_face_tim: invalid vector pointers "
                f"first=0x{first:X} last=0x{last:X} stride={stride}"
            )

        out["advanceFrameValid"] = bool(_read_u8(handle, impl + advance_valid_offset))
        out["lastAdvanceQueryFrame"] = _read_u32(handle, impl + last_query_frame_offset)
        out["queue"] = {
            "first": f"0x{first:X}",
            "last": f"0x{last:X}",
            "end": f"0x{end:X}",
            "count": count,
            "entries": entries,
        }

        atlas_cfg = cfg.get("atlas") or {}
        if atlas_cfg:
            atlas_addr = base + _parse_int(atlas_cfg["rva"])
            loaded_off = _parse_int(atlas_cfg.get("loaded_count_offset", 0))
            tpage_count_off = atlas_cfg.get("tpage_count_offset")
            atlas: dict[str, Any] = {
                "address": f"0x{atlas_addr:X}",
                "loadedCount": struct.unpack(
                    "<i", _read_bytes(handle, atlas_addr + loaded_off, 4)
                )[0],
            }
            if tpage_count_off is not None:
                atlas["tpageCountApprox"] = struct.unpack(
                    "<i",
                    _read_bytes(handle, atlas_addr + _parse_int(tpage_count_off), 4),
                )[0]
            out["atlas"] = atlas
    except Exception as exc:
        errors.append(f"stage1_face_tim: {exc}")
    return out, errors


def _resolve_commands(args: argparse.Namespace, profile: dict[str, Any]) -> list[str]:
    if args.command:
        return args.command
    if args.commands:
        return [p.strip() for p in args.commands.split(",") if p.strip()]
    prof_commands = profile.get("commands")
    if isinstance(prof_commands, list) and prof_commands:
        return [str(c) for c in prof_commands]
    return DEFAULT_COMMANDS


def _sample_debug_server(client: DebugClient, commands: list[str]) -> dict[str, Any]:
    out: dict[str, Any] = {}
    for cmd in commands:
        raw = client.send_full(cmd)
        item: dict[str, Any] = {"raw": raw}
        if cmd == "json":
            try:
                item["json"] = json.loads(raw)
            except Exception:
                item["error"] = "json parse failed"
        kv = _parse_kv(raw)
        if kv:
            item["kv"] = kv
        out[cmd] = item
    return out


def _sample_memory(
    handle: int | None,
    modules: dict[str, int],
    fields: list[dict[str, Any]],
) -> tuple[dict[str, Any], list[str]]:
    values: dict[str, Any] = {}
    errors: list[str] = []
    if handle is None:
        return values, errors
    for field in fields:
        if not field.get("enabled", True):
            continue
        name = str(field.get("name") or "")
        if not name:
            errors.append("field without name skipped")
            continue
        try:
            values[name] = _read_field(handle, modules, field)
        except Exception as exc:
            errors.append(f"{name}: {exc}")
    return values, errors


def _sample_specials(
    handle: int | None,
    modules: dict[str, int],
    profile: dict[str, Any],
) -> tuple[dict[str, Any], list[str]]:
    values: dict[str, Any] = {}
    errors: list[str] = []
    face_tim, face_tim_errors = _sample_stage1_face_tim(handle, modules, profile)
    if face_tim:
        values["stage1FaceTim"] = face_tim
    errors.extend(face_tim_errors)
    return values, errors


def _emit(sample: dict[str, Any], out_file: Path | None) -> None:
    line = json.dumps(sample, ensure_ascii=False, separators=(",", ":"))
    print(line, flush=True)
    if out_file is not None:
        out_file.parent.mkdir(parents=True, exist_ok=True)
        with out_file.open("a", encoding="utf-8") as f:
            f.write(line + "\n")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--profile", type=Path, help="JSON profile with commands and memory fields")
    ap.add_argument("--process", default=None, help="process image name, default from profile or PaRappaWin.exe")
    ap.add_argument("--pid", type=int, help="explicit target process id")
    ap.add_argument("--host", default=None, help="DebugServer host")
    ap.add_argument("--port", type=int, default=None, help="DebugServer port")
    ap.add_argument("--commands", help="comma-separated DebugServer commands")
    ap.add_argument("--command", action="append", help="DebugServer command; may be repeated")
    ap.add_argument("--out", type=Path, help="append JSONL output file")
    ap.add_argument("--count", type=int, default=1, help="sample count; 0 means until Ctrl+C")
    ap.add_argument("--interval", type=float, default=0.1, help="seconds between samples")
    ap.add_argument("--once", action="store_true", help="equivalent to --count 1")
    ap.add_argument("--no-memory", action="store_true", help="skip ReadProcessMemory")
    args = ap.parse_args(argv)

    profile = _load_profile(args.profile)
    process_name = args.process or str(profile.get("process") or DEFAULT_PROCESS)
    dbg_cfg = profile.get("debug_server") or {}
    host = args.host or str(dbg_cfg.get("host") or "127.0.0.1")
    port = int(args.port or dbg_cfg.get("port") or 19790)
    commands = _resolve_commands(args, profile)
    fields = [
        dict(field)
        for field in (profile.get("fields") or [])
        if dict(field).get("enabled", True)
    ]
    special_memory_enabled = bool(
        (profile.get("stage1_face_tim") or {}).get("enabled", False)
    )
    count = 1 if args.once else int(args.count)

    pid = args.pid
    if pid is None:
        pids = _find_pids_by_image(process_name)
        if not pids and fields and not args.no_memory:
            print(f"ERROR: process not found: {process_name}", file=sys.stderr)
            return 2
        pid = pids[0] if pids else 0

    handle: int | None = None
    modules: dict[str, int] = {}
    if pid and not args.no_memory and (fields or special_memory_enabled):
        handle = _open_process(pid)
        modules = _module_bases(pid)

    client = DebugClient(host=host, port=port, timeout=2.0)
    emitted = 0
    try:
        while count == 0 or emitted < count:
            errors: list[str] = []
            debug = _sample_debug_server(client, commands)
            mem, mem_errors = _sample_memory(handle, modules, fields)
            errors.extend(mem_errors)
            specials, special_errors = _sample_specials(handle, modules, profile)
            errors.extend(special_errors)
            sample = {
                "ts": time.time(),
                "pid": pid,
                "process": process_name,
                "debugServer": {"host": host, "port": port},
                "debug": debug,
                "mem": mem,
            }
            if specials:
                sample["specials"] = specials
            if errors:
                sample["errors"] = errors
            _emit(sample, args.out)
            emitted += 1
            if count != 0 and emitted >= count:
                break
            time.sleep(max(0.0, args.interval))
    except KeyboardInterrupt:
        return 130
    finally:
        if handle is not None:
            kernel32.CloseHandle(handle)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
