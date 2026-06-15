#!/usr/bin/env python3
"""Export parser-grade Stage1 event records with resolved resources/text.

Reverse-engineering aid only.
This export exists to validate a future native Win-side parser against PSX
overlay data. It must not be used as a runtime replacement for reading
COMOD1.BIN / COMPO01.INT directly.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BINARY = REPO_ROOT.parent / "S1" / "COMOD1.BIN"
DEFAULT_COMPO = REPO_ROOT.parent / "S1" / "COMPO01.INT"
DEFAULT_OUTPUT = REPO_ROOT / "temp" / "ghidra_reports" / "COMOD1_event_table_export.json"

BASE_ADDR = 0x801C3870
TEXT_TABLE_PTRS_ADDR = 0x801CE804
PAIR_TABLE_ADDR = 0x801CE818
STREAM_DESC_ADDR = 0x801CE8D4
STREAM_TABLE_ADDR = 0x801D2D64
HUD_SLOT_DESC_ADDR = 0x801CF924

TEXT_TABLE_COUNT = (PAIR_TABLE_ADDR - TEXT_TABLE_PTRS_ADDR) // 4
PAIR_TABLE_COUNT = (STREAM_DESC_ADDR - PAIR_TABLE_ADDR) // 4
STREAM_COUNT = 9
STREAM_ENTRY_SIZE = 0x0C
EVENT_SIZE = 0x24
HUD_SLOT_DESC_SIZE = 0x0C
HUD_SLOT_COUNT = 25

INT_HEADER_SIZE = 0x2000
INT_SECTOR_SIZE = 0x800
INT_ENTRY_SIZE = 20

TEXT_TABLE_LANGUAGES = {
    0: "English",
    1: "Deutsch",
    2: "Francais",
    3: "Italiano",
    4: "Espanol",
}


def _hex8(value: int) -> str:
    return f"0x{value:02X}"


def _hex16(value: int) -> str:
    return f"0x{value:04X}"


def _hex32(value: int) -> str:
    return f"0x{value:08X}"


def _off(addr: int) -> int:
    return addr - BASE_ADDR


def _iter_int_blocks(buf: bytes):
    offset = 0
    size = len(buf)
    block_index = 0
    while offset + INT_HEADER_SIZE <= size:
        header = buf[offset : offset + INT_HEADER_SIZE]
        block_type, num_files, sectors = struct.unpack_from("<III", header, 0)
        if block_type == 0xFFFFFFFF:
            return
        entries = []
        for i in range(num_files):
            ent_off = 16 + i * INT_ENTRY_SIZE
            if ent_off + INT_ENTRY_SIZE > INT_HEADER_SIZE:
                break
            entry_size = struct.unpack_from("<I", header, ent_off)[0]
            raw_name = header[ent_off + 4 : ent_off + 20]
            name = raw_name.split(b"\x00", 1)[0].decode("ascii", errors="ignore")
            entries.append((i, name, entry_size))
        yield block_index, block_type, entries
        offset += INT_HEADER_SIZE + sectors * INT_SECTOR_SIZE
        block_index += 1


def _load_mem_handle_map(compo_path: Path) -> dict[int, dict[str, object]]:
    blocks = list(_iter_int_blocks(compo_path.read_bytes()))
    if len(blocks) < 3 or blocks[2][1] != 3:
        raise RuntimeError("COMPO01.INT does not contain expected block2_Mem layout")
    handle_map = {}
    for mem_index, name, size in blocks[2][2]:
        handle = mem_index + 2
        handle_map[handle] = {
            "handle": _hex16(handle),
            "mem_index": mem_index,
            "name": name,
            "size": size,
        }
    return handle_map


def _read_cstr(blob: bytes, addr: int) -> str | None:
    if addr == 0:
        return None
    off = _off(addr)
    if off < 0 or off >= len(blob):
        return None
    out = bytearray()
    while off < len(blob):
        b = blob[off]
        if b == 0:
            break
        out.append(b)
        off += 1
    raw = bytes(out)
    for enc in ("ascii", "shift_jis", "latin1"):
        try:
            return raw.decode(enc)
        except Exception:
            pass
    return raw.decode("latin1", errors="replace")


def _read_text_tables(blob: bytes) -> list[dict[str, object]]:
    ptrs = [struct.unpack_from("<I", blob, _off(TEXT_TABLE_PTRS_ADDR) + i * 4)[0] for i in range(TEXT_TABLE_COUNT)]
    boundaries = ptrs[1:] + [TEXT_TABLE_PTRS_ADDR]
    tables = []
    for i, start in enumerate(ptrs):
        end = boundaries[i]
        count = (end - start) // 4
        entries = []
        for text_id in range(count):
            ptr = struct.unpack_from("<I", blob, _off(start) + text_id * 4)[0]
            entries.append(
                {
                    "text_id": text_id,
                    "ptr": _hex32(ptr) if ptr else None,
                    "text": _read_cstr(blob, ptr),
                }
            )
        tables.append(
            {
                "table_index": i,
                "language_guess": TEXT_TABLE_LANGUAGES.get(i),
                "addr": _hex32(start),
                "entry_count": count,
                "entries": entries,
            }
        )
    return tables


def _read_pairs(blob: bytes, handle_map: dict[int, dict[str, object]]) -> list[dict[str, object]]:
    pairs = []
    for pair_index in range(PAIR_TABLE_COUNT):
        addr = PAIR_TABLE_ADDR + pair_index * 4
        raw_a, raw_b = struct.unpack_from("<hh", blob, _off(addr))
        id_a = raw_a & 0xFFFF
        id_b = raw_b & 0xFFFF
        a = handle_map.get(id_a)
        b = handle_map.get(id_b)
        pairs.append(
            {
                "pair_index": pair_index,
                "addr": _hex32(addr),
                "idA": _hex16(id_a),
                "idB": _hex16(id_b),
                "nameA": a["name"] if a else None,
                "nameB": b["name"] if b else None,
            }
        )
    return pairs


def _pair_lookup(pairs: list[dict[str, object]], pair_index: int) -> dict[str, object] | None:
    if 0 <= pair_index < len(pairs):
        p = pairs[pair_index]
        return {
            "pair_index": pair_index,
            "idA": p["idA"],
            "nameA": p["nameA"],
            "idB": p["idB"],
            "nameB": p["nameB"],
        }
    return None


def _stream_desc(blob: bytes, stream_id: int) -> list[int]:
    return list(blob[_off(STREAM_DESC_ADDR + stream_id * 8) : _off(STREAM_DESC_ADDR + stream_id * 8) + 8])


def _stream_desc_rows(blob: bytes, handle_map: dict[int, dict[str, object]]) -> list[dict[str, object]]:
    rows = []
    for row_id in range(9):
        data = _stream_desc(blob, row_id)
        rows.append(
            {
                "row_id": row_id,
                "bytes": [_hex8(v) for v in data],
                "resource_names": [handle_map.get(v, {}).get("name") if v != 0 else None for v in data],
            }
        )
    return rows


def _hud_slot(slot_id: int) -> dict[str, object] | None:
    if slot_id == 0:
        return None
    return {
        "slot_id": slot_id,
        "desc_ptr_addr": _hex32(HUD_SLOT_DESC_ADDR + slot_id * HUD_SLOT_DESC_SIZE),
    }


def _text_by_id(text_tables: list[dict[str, object]], text_id: int) -> list[dict[str, object]]:
    out = []
    if text_id == 0:
        return out
    for table in text_tables:
        if text_id < table["entry_count"]:
            entry = table["entries"][text_id]
            out.append(
                {
                    "table_index": table["table_index"],
                    "language_guess": table["language_guess"],
                    "text": entry["text"],
                    "ptr": entry["ptr"],
                }
            )
    return out


def _export_hud_slot_descs(blob: bytes, handle_map: dict[int, dict[str, object]]) -> list[dict[str, object]]:
    out = []
    for slot_id in range(HUD_SLOT_COUNT):
        addr = HUD_SLOT_DESC_ADDR + slot_id * HUD_SLOT_DESC_SIZE
        events_ptr, count, cursor = struct.unpack_from("<III", blob, _off(addr))
        events = []
        if events_ptr != 0 and count != 0:
            for index in range(count):
                event_addr = events_ptr + index * 12
                delta = struct.unpack_from("<I", blob, _off(event_addr))[0]
                ids = struct.unpack_from("<4H", blob, _off(event_addr) + 4)
                events.append(
                    {
                        "event_index": index,
                        "event_addr": _hex32(event_addr),
                        "delta_frames": delta,
                        "tim_ids": [
                            {
                                "handle": _hex16(tim_id),
                                "name": handle_map.get(tim_id, {}).get("name"),
                            }
                            for tim_id in ids
                            if tim_id != 0
                        ],
                    }
                )
        out.append(
            {
                "slot_id": slot_id,
                "addr": _hex32(addr),
                "events_ptr": _hex32(events_ptr) if events_ptr != 0 else None,
                "event_count": count,
                "cursor_initial": cursor,
                "events": events,
            }
        )
    return out


def _export_stream(blob: bytes, stream_id: int, pairs: list[dict[str, object]], text_tables: list[dict[str, object]]):
    stream_addr = STREAM_TABLE_ADDR + stream_id * STREAM_ENTRY_SIZE
    events_ptr, count, cursor = struct.unpack_from("<III", blob, _off(stream_addr))
    row = _stream_desc(blob, stream_id)
    events = []

    for index in range(count):
        event_addr = events_ptr + index * EVENT_SIZE
        off = _off(event_addr)
        frame, flags = struct.unpack_from("<II", blob, off)
        bez = struct.unpack_from("<hhhh", blob, off + 0x08)
        mode_pairs = list(blob[off + 0x10 : off + 0x14])
        cool_pair = blob[off + 0x14]
        noncool_pair = blob[off + 0x15]
        byte16 = blob[off + 0x16]
        slot0 = blob[off + 0x17]
        slot1 = list(blob[off + 0x18 : off + 0x1C])
        slot2 = blob[off + 0x1C]
        byte1d = blob[off + 0x1D]
        byte1e = blob[off + 0x1E]
        byte1f = blob[off + 0x1F]
        text_id = blob[off + 0x20]

        events.append(
            {
                "event_index": index,
                "event_addr": _hex32(event_addr),
                "frame": frame,
                "flags04": _hex32(flags),
                "byte16": _hex8(byte16),
                "byte16_dec": byte16,
                "byte17": _hex8(slot0),
                "byte17_dec": slot0,
                "byte18": _hex8(slot1[0]),
                "byte18_dec": slot1[0],
                "byte19": _hex8(slot1[1]),
                "byte19_dec": slot1[1],
                "byte1A": _hex8(slot1[2]),
                "byte1A_dec": slot1[2],
                "byte1B": _hex8(slot1[3]),
                "byte1B_dec": slot1[3],
                "byte1C": _hex8(slot2),
                "byte1C_dec": slot2,
                "byte1D": _hex8(byte1d),
                "byte1D_dec": byte1d,
                "byte1E": _hex8(byte1e),
                "byte1E_dec": byte1e,
                "byte1F": _hex8(byte1f),
                "byte1F_dec": byte1f,
                "byte20": _hex8(text_id),
                "byte20_dec": text_id,
                "bez_ids_by_mode": {
                    "mode0_cool": bez[0],
                    "mode1_good": bez[1],
                    "mode2_bad": bez[2],
                    "mode3_awfl": bez[3],
                },
                "stream_desc_row": [_hex8(value) for value in row],
                "stream_desc_usage": {
                    "desc0_slot_f8": {"handle": _hex8(row[0]) if row[0] else None},
                    "desc1_cool_f4": {"handle": _hex8(row[1]) if row[1] else None},
                    "desc2_noncool_f4": {"handle": _hex8(row[2]) if row[2] else None},
                },
                "selector_pairs": {
                    "mode0_cool_ev10": _pair_lookup(pairs, mode_pairs[0]),
                    "mode1_good_ev11": _pair_lookup(pairs, mode_pairs[1]),
                    "mode2_bad_ev12": _pair_lookup(pairs, mode_pairs[2]),
                    "mode3_awfl_ev13": _pair_lookup(pairs, mode_pairs[3]),
                    "cool_ev14": _pair_lookup(pairs, cool_pair),
                    "noncool_ev15": _pair_lookup(pairs, noncool_pair),
                },
                "hud_slots": {
                    "slot0_ev17": _hud_slot(slot0),
                    "slot1_mode0_ev18": _hud_slot(slot1[0]),
                    "slot1_mode1_ev19": _hud_slot(slot1[1]),
                    "slot1_mode2_ev1A": _hud_slot(slot1[2]),
                    "slot1_mode3_ev1B": _hud_slot(slot1[3]),
                    "slot2_ev1C": _hud_slot(slot2),
                },
                "text": {
                    "text_id": _hex8(text_id),
                    "resolved_by_language": _text_by_id(text_tables, text_id),
                },
            }
        )
    return {
        "stream_id": stream_id,
        "stream_entry_addr": _hex32(stream_addr),
        "events_addr": _hex32(events_ptr),
        "count": count,
        "cursor_initial": cursor,
        "stream_desc_row": [_hex8(value) for value in row],
        "events": events,
    }


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Export parser-grade COMOD1 Stage1 event tables.")
    parser.add_argument("--binary", type=Path, default=DEFAULT_BINARY)
    parser.add_argument("--compo", type=Path, default=DEFAULT_COMPO)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--stream", type=int, action="append", default=[], help="Only export selected stream id(s).")
    return parser


def main() -> int:
    parser = _build_parser()
    args = parser.parse_args()

    blob = args.binary.read_bytes()
    handle_map = _load_mem_handle_map(args.compo)
    pairs = _read_pairs(blob, handle_map)
    text_tables = _read_text_tables(blob)
    streams = args.stream or list(range(1, STREAM_COUNT))

    out = {
        "analysis_only": True,
        "not_runtime_data_source": True,
        "binary": str(args.binary),
        "compo": str(args.compo),
        "base_addr": _hex32(BASE_ADDR),
        "text_table_ptrs_addr": _hex32(TEXT_TABLE_PTRS_ADDR),
        "pair_table_addr": _hex32(PAIR_TABLE_ADDR),
        "stream_desc_addr": _hex32(STREAM_DESC_ADDR),
        "stream_table_addr": _hex32(STREAM_TABLE_ADDR),
        "hud_slot_desc_addr": _hex32(HUD_SLOT_DESC_ADDR),
        "text_tables": [
            {
                "table_index": table["table_index"],
                "language_guess": table["language_guess"],
                "addr": table["addr"],
                "entry_count": table["entry_count"],
                "sample_1": table["entries"][1]["text"] if table["entry_count"] > 1 else None,
            }
            for table in text_tables
        ],
        "pair_table": pairs,
        "stream_desc_rows": _stream_desc_rows(blob, handle_map),
        "late_branch_ctx_layout": {
            "selector_bytes_addr_range": "ctx+0x122..0x126",
            "selector_ptrs_addr_range": "ctx+0x134..0x144",
            "selector_count": 5,
            "selector_source": "row[3..7] from selected row in unk_801CE8D4",
            "ptr_source": "dword_80091858[handle]",
            "consumer": "FUN_801cbfdc loops over ctx+0x134..0x144 and calls sub_8001AFD8(...)",
        },
        "late_branch_row_selector": {
            "row1_if_ctx_flags_0x20": 1,
            "row6_if_ctx_flags_0x2000_and_ctx_plus_0x72_lt_2": 6,
            "row8_if_ctx_flags_0x2000_and_ctx_plus_0x72_gte_2": 8,
            "row4_if_ctx_flags_0x40_and_global_0x8008ED24_ne_1": 4,
            "row5_if_ctx_flags_0x40_and_global_0x8008ED24_eq_1": 5,
            "row7_if_ctx_flags_0x4000": 7,
        },
        "late_branch_inference": {
            "ctx_flag_0x20": "default/base location TOD row (row1 -> ON_LOC/PA_HILOC/PA_LOC + TORO/TENNJOU/KABE*)",
            "ctx_flag_0x40": "secondary branch; row4 or row5 depending global 0x8008ED24",
            "ctx_flag_0x2000": "upper-grade route; row6/row8 split by ctx+0x72 threshold",
            "ctx_flag_0x4000": "alternate branch that currently resolves to the same resource row as row5 (ON_TI/PA_TI)",
            "ctx_plus_0x72": "main-exe-managed counter/level used to choose between ON_TO/PA_TO and ON_TOA/PA_TOA",
            "global_0x8008ED24": "main-exe-managed latch that disambiguates row4 vs row5 under ctx flag 0x40",
        },
        "ctx_flag_lifecycle_inference": {
            "0x20": {
                "set_by": "overlay FUN_801c895c when event.flags04 & 0x20",
                "consumed_by": "FUN_801c895c late branch -> row1",
                "clear_path": "no dedicated clear observed; reset by ctx reinit/zero path",
            },
            "0x40": {
                "set_by": "main-exe sub_80024FD0 under score/feedback branch",
                "consumed_by": "FUN_801c895c late branch -> row4/row5",
                "clear_path": "no dedicated clear observed; treated as short-lived latch until later reset/overwrite",
            },
            "0x2000": {
                "set_by": "main-exe sub_80024FD0 on GOOD->COOL route",
                "consumed_by": "FUN_801c895c late branch -> row6/row8 and FUN_801cbfdc render path",
                "clear_path": "no dedicated clear observed in overlay; likely reset with ctx lifecycle",
            },
            "0x4000": {
                "set_by": "main-exe sub_80024FD0 when dword_8008ED00 advances into branch 5",
                "consumed_by": "FUN_801c895c late branch -> row7",
                "clear_path": "no dedicated clear observed in overlay; likely reset with ctx lifecycle",
            },
        },
        "stream_lifecycle_inference": {
            "main_gameplay": {
                "stream_id": 1,
                "role": "main gameplay lyrics/prompts",
                "evidence": "largest stream; broad textId coverage; unique tail event #280 with flags04=0x80",
            },
            "rating_branch_after_stream1_tail": {
                "trigger": "stream1 event_index=280 frame=24576 flags04=0x80 -> dword_8008ED1C latch",
                "dispatch_by_ctx_mode": {
                    "0_COOL": 3,
                    "1_GOOD": 2,
                    "2_BAD": 4,
                    "3_AWFL": 4,
                },
                "notes": "FUN_801c9094 clears dword_8008ED1C then dispatches to stream2/3/4 and resets text cursors.",
            },
            "upper_grade_branch": {
                "trigger_flag": "ctx->flags & 0x2000",
                "dispatch": {
                    "ctx_plus_0x72_lt_2": 6,
                    "ctx_plus_0x72_gte_2": 8,
                },
                "notes": "GOOD->COOL path in main exe increments ctx+0x72 and chooses between stream6 and stream8.",
            },
            "late_branch_short_flow": {
                "trigger_flag": "ctx->flags & 0x4000",
                "dispatch_stream": 7,
                "notes": "row7 currently resolves to same ON_TI/PA_TI resource family as row5.",
            },
            "unresolved_stream": {
                "stream_id": 5,
                "status": "no direct dispatch path located yet",
                "notes": "content overlaps stream4, likely alternate branch or unused duplicate pending further evidence.",
            },
        },
        "known_flags04_bits": {
            "0x00000008": {
                "status": "confirmed",
                "effect": "desc[0] -> ctx+0xF8",
                "notes": "stream-row default selector path",
            },
            "0x00000020": {
                "status": "confirmed",
                "effect": "ctx->flags |= 0x20",
                "notes": "late-branch base location latch; leads to row1 consumption",
            },
            "0x00000080": {
                "status": "confirmed_by_lifecycle",
                "effect": "triggers dword_8008ED1C branch and rating-based stream dispatch",
                "notes": "currently only seen on stream1 tail event #280",
            },
            "0x00000100": {
                "status": "observed_only",
                "effect": "short-branch terminal marker candidate",
                "notes": "seen at tail of short streams 2/3/4/5; direct consumer not isolated yet",
            },
            "0x00000800": {
                "status": "observed_only",
                "effect": "unresolved",
                "notes": "seen in stream1; no selector payload and no isolated consumer yet",
            },
            "0x00010000": {
                "status": "confirmed",
                "effect": "ev+0x14/0x15 -> pair table -> ctx+0xDC/E8",
                "notes": "COOL vs non-COOL pair selection",
            },
            "0x00020000": {
                "status": "confirmed",
                "effect": "ev+0x10..0x13 -> pair table -> ctx+0xE0/EC",
                "notes": "mode-dependent pair selection for COOL/GOOD/BAD/AWFL",
            },
            "0x00030000": {
                "status": "confirmed_composite",
                "effect": "0x10000 + 0x20000 together",
                "notes": "main gameplay stream frequently uses both persistent pair paths together",
            },
            "0x00031000": {
                "status": "observed_only",
                "effect": "0x00030000 plus unresolved 0x1000 bit",
                "notes": "seen in stream1; 0x1000 sub-bit still unresolved",
            },
            "0x00040000": {
                "status": "confirmed",
                "effect": "desc[1/2] -> ctx+0xF4",
                "notes": "COOL vs non-COOL TOD selector from stream desc row",
            },
            "0x000F1020": {
                "status": "observed_composite",
                "effect": "intro/setup composite",
                "notes": "appears at frame 0 of short streams; includes 0x20 plus unresolved high bits",
            },
            "0x00100000": {
                "status": "observed_only",
                "effect": "unresolved",
                "notes": "seen in stream1 with no direct selector payload",
            },
        },
        "hud_slot_descs": _export_hud_slot_descs(blob, handle_map),
        "streams": [_export_stream(blob, stream_id, pairs, text_tables) for stream_id in streams],
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(out, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(f"binary: {args.binary}")
    print(f"compo:  {args.compo}")
    print(f"output: {args.output}")
    print(f"streams exported: {len(out['streams'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
