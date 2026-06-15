#!/usr/bin/env python3
"""
Read-only external sampler for Stage1 scene-submit state.

This sidecar intentionally stays outside PaRappaWin.exe. It reads the current
process with ReadProcessMemory and samples DebugServer commands, but never
injects code or writes target memory.
"""

from __future__ import annotations

import argparse
import json
import struct
import time
from pathlib import Path
from typing import Any

from stage1_mem_probe import (  # noqa: E402
    DEFAULT_PROCESS,
    DebugClient,
    _find_pids_by_image,
    _module_bases,
    _open_process,
    _parse_kv,
    _read_bytes,
    _read_u8,
    _read_u32,
    _read_u64,
    kernel32,
)


# PDB-derived offsets for the current full build. Override with CLI flags if a
# later full rebuild moves the owner/static RVAs.
DEFAULT_SCENE_RUNTIME_RVA = 0x00592928
DEFAULT_428B0_DEBUG_RVA = 0x00592790

OFF_IMPL_PTR = 0
OFF_LOCAL_DESC_STATES = 0
OFF_ADVANCE_FRAME_VALID = 20528
OFF_LAST_ADVANCE_QUERY_FRAME = 20532
OFF_VIEW_ROWS = 20536
OFF_PROJECTION = 20584
OFF_CAMERA_AUTHORITY = 34004
OFF_CAMERA_PRESENTATION = 34048
OFF_CAMERA_BEZ_ID = 34092
OFF_CAMERA_SOURCE_EVENT = 34096
OFF_CAMERA_USING_NEUTRAL = 34100
OFF_CAMERA_ADVANCE_VALID = 34101
OFF_LAST_CAMERA_QUERY_FRAME = 34104
OFF_SUBMIT_RANGES_801CAC34 = 258112
OFF_SUBMIT_RANGES_801CB190 = 258136
OFF_PSX_GLOBALS = 258204
OFF_PSX_DWORD_801D7F40 = 0
OFF_PSX_DWORD_801D7F44 = 4
OFF_PSX_DWORD_801D7F48 = 8
OFF_PSX_DWORD_801D7F4C = 12
OFF_PSX_DWORD_801D7F50 = 16
OFF_PSX_UI_LIST_INIT_COUNTER = 20
OFF_PSX_UI_LIST_BLOCK = 24
OFF_PSX_UI_RENDER_BLOCK = 25
OFF_PSX_UI_RENDER_BLOCK2 = 26
OFF_PSX_UI_RENDER_REFRESH_REQUEST = 27
OFF_PSX_UI_LIST_A_ACTIVE = 28
OFF_PSX_UI_LIST_B_ACTIVE = 29
OFF_PSX_UI_LIST_C_ACTIVE = 30
OFF_PSX_UI_LIST_D_ACTIVE = 31
OFF_PSX_UI_LIST_A_ITEM_COUNT = 32
OFF_PSX_UI_LIST_B_ITEM_COUNT = 36
OFF_PSX_UI_LIST_C_ITEM_COUNT = 40
OFF_PSX_UI_LIST_D_ITEM_COUNT = 44
OFF_PSX_UI_LIST_A_SEQ = 48
OFF_PSX_UI_LIST_B_SEQ = 52
OFF_PSX_UI_LIST_C_SEQ = 56
OFF_PSX_UI_LIST_D_SEQ = 60
OFF_PSX_DWORD_801D3014 = 64
OFF_PSX_DWORD_801D3018 = 68
OFF_PSX_DWORD_801D301C = 72
OFF_PSX_DWORD_801D3020 = 76
OFF_PSX_DWORD_801D3024 = 80
OFF_PSX_DWORD_801D302C = 84
OFF_PSX_DWORD_801D3034 = 88
OFF_PSX_DWORD_801D3028 = 92

DESC_STATE_STRIDE = 56
SUBMIT_RANGE_STRIDE = 24

WORK_NAMES = {
    0: "W7B1C",
    1: "W80A8",
    2: "W8080",
    3: "W8058",
    4: "W526C",
    5: "W3244",
    6: "W5AF4",
}

DESC_NAMES = {
    0x801D7CF8: "NIN_TEN",
    0x801D7D08: "NIN_TTM",
    0x801D7D18: "NIN_SHJ",
    0x801D7D28: "NIN_L",
    0x801D7D38: "NIN_R",
    0x801D7EF0: "WAKU_L",
    0x801D7F00: "WAKU_R",
    0x801D7F10: "DONDE_L",
    0x801D7F20: "DONDE_R",
    0x801D7F68: "TOUROU",
    0x801D7F78: "TENNJOU",
    0x801D7F88: "KABE_R",
    0x801D7F98: "KABE_L",
    0x801D7FA8: "KABE_B",
    0x801D7FB8: "KAMON_L",
    0x801D7FC8: "KAMON_R",
    0x801D7FD8: "MADO_L",
    0x801D7FE8: "MADO_R",
    0x801D7FF8: "AUX_7FF8",
    0x801D8008: "AUX_8008",
    0x801D8018: "AUX_8018",
    0x801D8028: "KAGE",
    0x801D8038: "FLOOR",
    0x801D98D0: "TAMA",
    0x801D98F0: "ON",
    0x801DB2D8: "PA",
}

DEFAULT_COMMANDS = ["status", "json", "stage1ovl scene428b0"]


def _hex32(value: int) -> str:
    return f"0x{value:08X}"


def _hex64(value: int) -> str:
    return f"0x{value:X}"


def _desc_name(desc_addr: int) -> str | None:
    name = DESC_NAMES.get(desc_addr)
    if name is not None:
        return name
    if 0x801D98D0 <= desc_addr <= 0x801D98F0 and (desc_addr - 0x801D98D0) % 16 == 0:
        return f"TAMA_{(desc_addr - 0x801D98D0) // 16}"
    return None


def _read_vec_header(handle: int, address: int) -> tuple[int, int, int]:
    return struct.unpack("<QQQ", _read_bytes(handle, address, 24))


def _vector_info(
    handle: int,
    impl: int,
    offset: int,
    stride: int,
    max_entries: int,
    label: str,
) -> tuple[dict[str, Any], bytes, list[str]]:
    errors: list[str] = []
    first, last, end = _read_vec_header(handle, impl + offset)
    raw_count_bytes = last - first if last >= first else -1
    count = 0
    data = b""
    if first and last >= first and raw_count_bytes % stride == 0:
        count = raw_count_bytes // stride
        read_count = min(count, max_entries)
        if count > max_entries:
            errors.append(f"{label}: count {count} capped to {max_entries}")
        if read_count:
            data = _read_bytes(handle, first, read_count * stride)
    elif first or last or end:
        errors.append(
            f"{label}: invalid vector pointers first=0x{first:X} "
            f"last=0x{last:X} stride={stride}"
        )
    return (
        {
            "first": _hex64(first),
            "last": _hex64(last),
            "end": _hex64(end),
            "count": count,
            "sampledCount": len(data) // stride,
        },
        data,
        errors,
    )


def _decode_submit_range(data: bytes, index: int) -> dict[str, Any]:
    off = index * SUBMIT_RANGE_STRIDE
    work = data[off]
    depth = data[off + 1]
    head_insert = bool(data[off + 2])
    base_addr = struct.unpack_from("<I", data, off + 4)[0]
    slot = data[off + 8]
    work_addr = struct.unpack_from("<I", data, off + 12)[0]
    desc_addr = struct.unpack_from("<I", data, off + 16)[0]
    desc_count = data[off + 20]
    return {
        "index": index,
        "work": WORK_NAMES.get(work, f"work{work}"),
        "workId": work,
        "depth": depth,
        "headInsert": head_insert,
        "psxWorkBaseAddr": _hex32(base_addr),
        "slot8006EDA8": slot,
        "psxWorkAddr": _hex32(work_addr),
        "descAddr": _hex32(desc_addr),
        "descName": _desc_name(desc_addr),
        "descCount": desc_count,
    }


def _decode_desc_state(data: bytes, index: int) -> dict[str, Any]:
    off = index * DESC_STATE_STRIDE
    desc_addr = struct.unpack_from("<I", data, off + 0)[0]
    attr = struct.unpack_from("<I", data, off + 4)[0]
    coord_node_addr = struct.unpack_from("<I", data, off + 8)[0]
    object_desc_token = struct.unpack_from("<I", data, off + 12)[0]
    model_set = data[off + 16]
    model_index = struct.unpack_from("<H", data, off + 18)[0]
    object_index = struct.unpack_from("<H", data, off + 20)[0]
    coord_valid = bool(data[off + 22])
    tx = struct.unpack_from("<i", data, off + 44)[0]
    ty = struct.unpack_from("<i", data, off + 48)[0]
    tz = struct.unpack_from("<i", data, off + 52)[0]
    return {
        "index": index,
        "descAddr": _hex32(desc_addr),
        "descName": _desc_name(desc_addr),
        "attr": _hex32(attr),
        "coordNodeAddr": _hex32(coord_node_addr),
        "objectDescToken8004274C": _hex32(object_desc_token),
        "modelSet": model_set,
        "modelIndex": model_index,
        "objectIndex8004274C": object_index,
        "coordValid": coord_valid,
        "coordT": [tx, ty, tz],
    }


def _decode_camera(data: bytes) -> dict[str, Any]:
    return {
        "valid": bool(data[0]),
        "pos": list(struct.unpack_from("<fff", data, 4)),
        "target": list(struct.unpack_from("<fff", data, 16)),
        "twist": struct.unpack_from("<i", data, 28)[0],
        "projectionDistance": struct.unpack_from("<i", data, 32)[0],
        "screenCenterX": struct.unpack_from("<i", data, 36)[0],
        "screenCenterY": struct.unpack_from("<i", data, 40)[0],
    }


def _decode_scene_428b0_debug(handle: int, address: int) -> dict[str, Any]:
    data = _read_bytes(handle, address, 224)
    return {
        "address": _hex64(address),
        "drawCalled": bool(data[0]),
        "rendererAvailable": bool(data[1]),
        "commandCount": struct.unpack_from("<I", data, 4)[0],
        "validCount": struct.unpack_from("<I", data, 8)[0],
        "gapCount": struct.unpack_from("<I", data, 12)[0],
        "specialTableCount": struct.unpack_from("<I", data, 16)[0],
        "lastKnown": bool(data[20]),
        "lastSpecialKnown": bool(data[212]),
    }


def _range_summary(ranges: list[dict[str, Any]]) -> dict[str, Any]:
    by_desc: dict[str, int] = {}
    ordered: list[str] = []
    for item in ranges:
        name = item.get("descName") or item["descAddr"]
        by_desc[name] = by_desc.get(name, 0) + int(item.get("descCount") or 0)
        ordered.append(
            f"{item['index']}:{name}:{item['work']}:d{item['depth']}:c{item['descCount']}"
        )
    return {"byDescCount": by_desc, "order": ordered}


def _sample_debug_server(host: str, port: int, commands: list[str]) -> dict[str, Any]:
    client = DebugClient(host=host, port=port, timeout=2.0)
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


def _sample_scene_submit(
    handle: int,
    base: int,
    runtime_rva: int,
    debug_rva: int,
    max_desc: int,
    max_ranges: int,
) -> tuple[dict[str, Any], list[str]]:
    errors: list[str] = []
    runtime_addr = base + runtime_rva
    debug_addr = base + debug_rva
    impl = _read_u64(handle, runtime_addr + OFF_IMPL_PTR)
    out: dict[str, Any] = {
        "moduleBase": _hex64(base),
        "runtimeAddress": _hex64(runtime_addr),
        "impl": _hex64(impl),
    }
    if impl == 0:
        out["initialized"] = False
        return out, errors
    out["initialized"] = True

    try:
        out["advanceFrameValid"] = bool(
            _read_u8(handle, impl + OFF_ADVANCE_FRAME_VALID)
        )
        out["lastAdvanceQueryFrame"] = _read_u32(
            handle, impl + OFF_LAST_ADVANCE_QUERY_FRAME
        )
        out["lastCameraQueryFrame801CBFDC"] = _read_u32(
            handle, impl + OFF_LAST_CAMERA_QUERY_FRAME
        )
        out["cameraBezId801CBFDC"] = struct.unpack(
            "<h", _read_bytes(handle, impl + OFF_CAMERA_BEZ_ID, 2)
        )[0]
        out["cameraSourceEventPsxAddr801CBFDC"] = _hex32(
            _read_u32(handle, impl + OFF_CAMERA_SOURCE_EVENT)
        )
        out["cameraUsingNeutral801CBFDC"] = bool(
            _read_u8(handle, impl + OFF_CAMERA_USING_NEUTRAL)
        )
        out["cameraAdvanceFrameValid801CBFDC"] = bool(
            _read_u8(handle, impl + OFF_CAMERA_ADVANCE_VALID)
        )
        out["psx801CBFDC"] = {
            "dword_801D7F40": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D7F40)
            ),
            "dword_801D7F44": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D7F44)
            ),
            "dword_801D7F48": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D7F48)
            ),
            "dword_801D7F4C": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D7F4C)
            ),
            "dword_801D7F50": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D7F50)
            ),
            "g_PrStageRunner_UIList_InitCounter": struct.unpack(
                "<i",
                _read_bytes(
                    handle,
                    impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_INIT_COUNTER,
                    4,
                ),
            )[0],
            "g_PrStageRunner_UIList_Block": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_BLOCK)
            ),
            "g_PrStageRunner_UIRender_Block": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_RENDER_BLOCK)
            ),
            "g_PrStageRunner_UIRender_Block2": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_RENDER_BLOCK2)
            ),
            "g_PrStageRunner_UIRender_RefreshRequest": bool(
                _read_u8(
                    handle,
                    impl + OFF_PSX_GLOBALS + OFF_PSX_UI_RENDER_REFRESH_REQUEST,
                )
            ),
            "g_PrStageRunner_UIListA_Active": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_A_ACTIVE)
            ),
            "g_PrStageRunner_UIListB_Active": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_B_ACTIVE)
            ),
            "g_PrStageRunner_UIListC_Active": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_C_ACTIVE)
            ),
            "g_PrStageRunner_UIListD_Active": bool(
                _read_u8(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_D_ACTIVE)
            ),
            "g_PrStageRunner_UIListA_ItemCount": struct.unpack(
                "<i",
                _read_bytes(
                    handle,
                    impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_A_ITEM_COUNT,
                    4,
                ),
            )[0],
            "g_PrStageRunner_UIListB_ItemCount": struct.unpack(
                "<i",
                _read_bytes(
                    handle,
                    impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_B_ITEM_COUNT,
                    4,
                ),
            )[0],
            "g_PrStageRunner_UIListC_ItemCount": struct.unpack(
                "<i",
                _read_bytes(
                    handle,
                    impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_C_ITEM_COUNT,
                    4,
                ),
            )[0],
            "g_PrStageRunner_UIListD_ItemCount": struct.unpack(
                "<i",
                _read_bytes(
                    handle,
                    impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_D_ITEM_COUNT,
                    4,
                ),
            )[0],
            "g_PrStageRunner_UIListA_Seq": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_A_SEQ
            ),
            "g_PrStageRunner_UIListB_Seq": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_B_SEQ
            ),
            "g_PrStageRunner_UIListC_Seq": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_C_SEQ
            ),
            "g_PrStageRunner_UIListD_Seq": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_UI_LIST_D_SEQ
            ),
            "dword_801D3014": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D3014
            ),
            "dword_801D3018": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D3018
            ),
            "dword_801D301C": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D301C
            ),
            "dword_801D3020": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D3020
            ),
            "dword_801D3024": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D3024
            ),
            "dword_801D302C": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D302C)
            ),
            "dword_801D3034": _hex32(
                _read_u32(handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D3034)
            ),
            "dword_801D3028": _read_u32(
                handle, impl + OFF_PSX_GLOBALS + OFF_PSX_DWORD_801D3028
            ),
        }
    except Exception as exc:
        errors.append(f"header: {exc}")

    try:
        view_rows = struct.unpack(
            "<12i", _read_bytes(handle, impl + OFF_VIEW_ROWS, 48)
        )
        out["viewRows"] = list(view_rows)
    except Exception as exc:
        errors.append(f"viewRows: {exc}")

    try:
        projection = struct.unpack(
            "<3i", _read_bytes(handle, impl + OFF_PROJECTION, 12)
        )
        out["projection"] = {
            "projectionDistance": projection[0],
            "screenCenterX": projection[1],
            "screenCenterY": projection[2],
        }
    except Exception as exc:
        errors.append(f"projection: {exc}")

    try:
        out["cameraAuthority"] = _decode_camera(
            _read_bytes(handle, impl + OFF_CAMERA_AUTHORITY, 44)
        )
        out["cameraPresentation"] = _decode_camera(
            _read_bytes(handle, impl + OFF_CAMERA_PRESENTATION, 44)
        )
    except Exception as exc:
        errors.append(f"camera: {exc}")

    try:
        desc_info, desc_data, desc_errors = _vector_info(
            handle,
            impl,
            OFF_LOCAL_DESC_STATES,
            DESC_STATE_STRIDE,
            max_desc,
            "localDescStates",
        )
        errors.extend(desc_errors)
        desc_entries = [
            _decode_desc_state(desc_data, i)
            for i in range(len(desc_data) // DESC_STATE_STRIDE)
        ]
        out["localDescStates"] = desc_info | {"entries": desc_entries}
    except Exception as exc:
        errors.append(f"localDescStates: {exc}")

    for label, offset in (
        ("submitRanges801CAC34", OFF_SUBMIT_RANGES_801CAC34),
        ("submitRanges801CB190", OFF_SUBMIT_RANGES_801CB190),
    ):
        try:
            range_info, range_data, range_errors = _vector_info(
                handle,
                impl,
                offset,
                SUBMIT_RANGE_STRIDE,
                max_ranges,
                label,
            )
            errors.extend(range_errors)
            ranges = [
                _decode_submit_range(range_data, i)
                for i in range(len(range_data) // SUBMIT_RANGE_STRIDE)
            ]
            out[label] = range_info | {
                "entries": ranges,
                "summary": _range_summary(ranges),
            }
        except Exception as exc:
            errors.append(f"{label}: {exc}")

    try:
        out["scene428B0Debug"] = _decode_scene_428b0_debug(handle, debug_addr)
    except Exception as exc:
        errors.append(f"scene428B0Debug: {exc}")

    return out, errors


def _emit(sample: dict[str, Any], out_file: Path | None) -> None:
    line = json.dumps(sample, ensure_ascii=False, separators=(",", ":"))
    print(line, flush=True)
    if out_file is not None:
        out_file.parent.mkdir(parents=True, exist_ok=True)
        with out_file.open("a", encoding="utf-8") as f:
            f.write(line + "\n")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--process", default=DEFAULT_PROCESS)
    ap.add_argument("--pid", type=int)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=19790)
    ap.add_argument("--runtime-rva", type=lambda v: int(v, 0), default=DEFAULT_SCENE_RUNTIME_RVA)
    ap.add_argument("--debug-rva", type=lambda v: int(v, 0), default=DEFAULT_428B0_DEBUG_RVA)
    ap.add_argument("--max-desc", type=int, default=96)
    ap.add_argument("--max-ranges", type=int, default=96)
    ap.add_argument("--command", action="append")
    ap.add_argument("--out", type=Path)
    ap.add_argument("--count", type=int, default=1)
    ap.add_argument("--interval", type=float, default=0.1)
    ap.add_argument("--no-debug-server", action="store_true")
    args = ap.parse_args(argv)

    pid = args.pid
    if pid is None:
        pids = _find_pids_by_image(args.process)
        if not pids:
            print(f"ERROR: process not found: {args.process}")
            return 2
        pid = pids[0]

    handle = _open_process(pid)
    modules = _module_bases(pid)
    base = modules.get(args.process.lower())
    if base is None:
        kernel32.CloseHandle(handle)
        print(f"ERROR: module not found: {args.process}")
        return 2

    commands = args.command or DEFAULT_COMMANDS
    emitted = 0
    try:
        while args.count == 0 or emitted < args.count:
            errors: list[str] = []
            scene, scene_errors = _sample_scene_submit(
                handle,
                base,
                args.runtime_rva,
                args.debug_rva,
                args.max_desc,
                args.max_ranges,
            )
            errors.extend(scene_errors)
            sample: dict[str, Any] = {
                "ts": time.time(),
                "pid": pid,
                "process": args.process,
                "sceneSubmit": scene,
            }
            if not args.no_debug_server:
                sample["debug"] = _sample_debug_server(args.host, args.port, commands)
            if errors:
                sample["errors"] = errors
            _emit(sample, args.out)
            emitted += 1
            if args.count != 0 and emitted >= args.count:
                break
            time.sleep(max(0.0, args.interval))
    finally:
        kernel32.CloseHandle(handle)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
