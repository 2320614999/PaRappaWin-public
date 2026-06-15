#!/usr/bin/env python3
"""Summarize COMOD1 Stage1 event streams, selector columns, and resource pairs.

Reverse-engineering aid only.
This script emits offline JSON views for analysis and parser validation.
It must not be used as a runtime data source for Win-side Stage1 behavior.
"""

from __future__ import annotations

import argparse
import json
import struct
from collections import Counter, defaultdict
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BINARY = REPO_ROOT.parent / "S1" / "COMOD1.BIN"
DEFAULT_COMPO = REPO_ROOT.parent / "S1" / "COMPO01.INT"
DEFAULT_OUTPUT = REPO_ROOT / "temp" / "ghidra_reports" / "COMOD1_event_stream_summary.json"

BASE_ADDR = 0x801C3870
TEXT_TABLE_PTRS_ADDR = 0x801CE804
STREAM_DESC_ADDR = 0x801CE8D4
PAIR_TABLE_ADDR = 0x801CE818
TEXT_TABLE_COUNT = (PAIR_TABLE_ADDR - TEXT_TABLE_PTRS_ADDR) // 4
PAIR_TABLE_COUNT = (STREAM_DESC_ADDR - PAIR_TABLE_ADDR) // 4
STREAM_TABLE_ADDR = 0x801D2D64
STREAM_ENTRY_SIZE = 0x0C
STREAM_COUNT = 9
EVENT_SIZE = 0x24

INT_HEADER_SIZE = 0x2000
INT_SECTOR_SIZE = 0x800
INT_ENTRY_SIZE = 20

ROLE_GUESSES = {
    1: "候选：主 gameplay 歌词/提示流；数量最大，且 textId 覆盖大段正篇歌词。",
    2: "候选：过关分支短对白流；textId 已见“Good job, Parappa / Ya hoo! Alright!!”。",
    3: "候选：过关祝贺对白流；textId 已见“Parappa / I'm so proud of you / Congratulations”。",
    4: "候选：失败/惊讶短反馈流A；textId 已见“Again / What!?”。",
    5: "候选：失败/惊讶短反馈流B；当前文本与 stream4 相同，可能是另一条件分支复用。",
    6: "候选：高表现/COOL praise 流；textId 已见“Man, you're so good!”。",
    7: "候选：低表现/补救提示流；textId 已见“You\\'re bad / super beginner\\'s course”。",
    8: "候选：COOL 上位/快速收束分支流；已见“You're pickin' up too fast! / I'm outta here!”。",
}

TEXT_TABLE_LANGUAGES = {
    0: "English",
    1: "Deutsch",
    2: "Francais",
    3: "Italiano",
    4: "Espanol",
}


def _hex32(value: int) -> str:
    return f"0x{value:08X}"


def _hex16(value: int) -> str:
    return f"0x{value:04X}"


def _hex8(value: int) -> str:
    return f"0x{value:02X}"


def _offset(addr: int) -> int:
    return addr - BASE_ADDR


def _read_u32(blob: bytes, addr: int) -> int:
    return struct.unpack_from("<I", blob, _offset(addr))[0]


def _read_row(blob: bytes, addr: int, size: int) -> bytes:
    start = _offset(addr)
    return blob[start : start + size]


def _read_cstr(blob: bytes, addr: int) -> str | None:
    if addr == 0:
        return None
    off = _offset(addr)
    if off < 0 or off >= len(blob):
        return None

    data = bytearray()
    while off < len(blob):
        b = blob[off]
        if b == 0:
            break
        data.append(b)
        off += 1

    raw = bytes(data)
    for encoding in ("ascii", "shift_jis", "latin1"):
        try:
            return raw.decode(encoding)
        except Exception:
            continue
    return raw.decode("latin1", errors="replace")


def _iter_int_blocks(buf: bytes):
    offset = 0
    size = len(buf)
    block_index = 0

    while offset + INT_HEADER_SIZE <= size:
        header = buf[offset : offset + INT_HEADER_SIZE]
        block_type, num_files, sectors = struct.unpack_from("<III", header, 0)
        if block_type == 0xFFFFFFFF:
            return

        block_data_off = offset + INT_HEADER_SIZE
        block_data_size = sectors * INT_SECTOR_SIZE
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
        offset += INT_HEADER_SIZE + block_data_size
        block_index += 1


def _load_mem_handle_map(compo_path: Path) -> dict[int, dict[str, object]]:
    buf = compo_path.read_bytes()
    blocks = list(_iter_int_blocks(buf))
    if len(blocks) < 3 or blocks[2][1] != 3:
        raise RuntimeError("COMPO01.INT does not contain expected block2_Mem layout")

    handle_map: dict[int, dict[str, object]] = {}
    for mem_index, name, size in blocks[2][2]:
        handle = mem_index + 2
        handle_map[handle] = {
            "handle": _hex16(handle),
            "mem_index": mem_index,
            "name": name,
            "size": size,
        }
    return handle_map


def _stream_desc_summary(row: bytes, handle_map: dict[int, dict[str, object]]) -> dict[str, object]:
    def name_for(byte_value: int) -> str | None:
        entry = handle_map.get(byte_value)
        return entry["name"] if entry else None

    return {
        "bytes": [_hex8(value) for value in row],
        "fun_801c895c_consumed": {
            "byte0": {
                "selector": "desc[0]",
                "semantics": "used when flags04 & 0x00000008; handle[row[0]] -> ctx+0xF8",
                "resource_name": name_for(row[0]),
            },
            "byte1": {
                "selector": "desc[1]",
                "semantics": "used when flags04 & 0x00040000 and ctx+0x4E==0(COOL); handle[row[1]] -> ctx+0xF4",
                "resource_name": name_for(row[1]),
            },
            "byte2": {
                "selector": "desc[2]",
                "semantics": "used when flags04 & 0x00040000 and ctx+0x4E!=0; handle[row[2]] -> ctx+0xF4",
                "resource_name": name_for(row[2]),
            },
        },
        "currently_undetermined_in_fun_801c895c": {
            f"byte{i}": {"value": _hex8(row[i]), "resource_name": name_for(row[i])}
            for i in range(3, 8)
        },
        "late_branch_bytes_3_7": {
            "semantics": "used by FUN_801c895c late branch @0x801C8E4C; row[3..7] copied into ctx+0x122..0x126 and resolved via handle table into ctx+0x134..0x144",
            "bytes": [_hex8(row[i]) for i in range(3, 8)],
            "resource_names": [name_for(row[i]) for i in range(3, 8)],
        },
    }


def _selector_semantics() -> dict[str, object]:
    return {
        "flags04_0x00040000": {
            "source": "stream desc row",
            "cool_column": "desc[1]",
            "noncool_column": "desc[2]",
            "target_slot": "ctx+0xF4",
        },
        "flags04_0x00010000": {
            "source": "event bytes",
            "cool_column": "ev+0x14",
            "noncool_column": "ev+0x15",
            "pair_lookup": "unk_801CE818[idx] -> {idA,idB}",
            "target_slots": ["ctx+0xDC", "ctx+0xE8"],
        },
        "flags04_0x00020000": {
            "source": "event bytes",
            "columns_by_mode": {
                "0_COOL": "ev+0x10",
                "1_GOOD": "ev+0x11",
                "2_BAD": "ev+0x12",
                "3_AWFL": "ev+0x13",
            },
            "pair_lookup": "unk_801CE818[idx] -> {idA,idB}",
            "target_slots": ["ctx+0xE0", "ctx+0xEC"],
        },
        "flags04_0x00000008": {
            "source": "stream desc row",
            "column": "desc[0]",
            "target_slot": "ctx+0xF8",
        },
        "event_hud_text_fields": {
            "ev+0x17": {
                "source": "event byte",
                "semantics": "slot0 hud overlay id",
                "target_globals": ["0x8008ED44", "0x8008ED48", "0x8008ED4C"],
                "desc_ptr_formula": "0x801CF924 + 12 * slotId",
            },
            "ev+0x18..0x1B": {
                "source": "event bytes",
                "semantics": "slot1 hud overlay id selected by ctx+0x4E mode 0..3",
                "target_globals": ["0x8008ED50", "0x8008ED54", "0x8008ED58"],
                "desc_ptr_formula": "0x801CF924 + 12 * slotId",
            },
            "ev+0x1C": {
                "source": "event byte",
                "semantics": "slot2 hud overlay id",
                "target_globals": ["0x8008ED5C", "0x8008ED60", "0x8008ED64"],
                "desc_ptr_formula": "0x801CF924 + 12 * slotId",
            },
            "ev+0x20": {
                "source": "event byte",
                "semantics": "textId consumed by FUN_801c8604",
                "target_slot": "ctx+0x10C",
                "lookup": "off_801CE804[ctx+0x66][textId]",
            },
        },
    }


def _read_pair_table(blob: bytes, handle_map: dict[int, dict[str, object]]) -> list[dict[str, object]]:
    pairs = []
    for pair_index in range(PAIR_TABLE_COUNT):
        addr = PAIR_TABLE_ADDR + pair_index * 4
        raw_a, raw_b = struct.unpack_from("<hh", blob, _offset(addr))
        id_a = raw_a & 0xFFFF
        id_b = raw_b & 0xFFFF
        entry_a = handle_map.get(id_a)
        entry_b = handle_map.get(id_b)
        pairs.append(
            {
                "pair_index": pair_index,
                "addr": _hex32(addr),
                "idA": _hex16(id_a),
                "idB": _hex16(id_b),
                "nameA": entry_a["name"] if entry_a else None,
                "nameB": entry_b["name"] if entry_b else None,
            }
        )
    return pairs


def _read_text_tables(blob: bytes) -> list[dict[str, object]]:
    table_ptrs = [
        struct.unpack_from("<I", blob, _offset(TEXT_TABLE_PTRS_ADDR) + index * 4)[0]
        for index in range(TEXT_TABLE_COUNT)
    ]
    boundaries = table_ptrs[1:] + [TEXT_TABLE_PTRS_ADDR]
    tables: list[dict[str, object]] = []

    for index, start in enumerate(table_ptrs):
        end = boundaries[index]
        entry_count = (end - start) // 4
        entries = []
        for text_id in range(entry_count):
            ptr = struct.unpack_from("<I", blob, _offset(start) + text_id * 4)[0]
            entries.append(
                {
                    "text_id": _hex8(text_id),
                    "ptr": _hex32(ptr) if ptr != 0 else None,
                    "text": _read_cstr(blob, ptr),
                }
            )
        tables.append(
            {
                "table_index": index,
                "language_guess": TEXT_TABLE_LANGUAGES.get(index),
                "addr": _hex32(start),
                "entry_count": entry_count,
                "sample_1": entries[1]["text"] if len(entries) > 1 else None,
                "entries": entries,
            }
        )
    return tables


def _pair_names(pair_table: list[dict[str, object]], pair_index: int) -> dict[str, object] | None:
    if 0 <= pair_index < len(pair_table):
        pair = pair_table[pair_index]
        return {
            "pair_index": pair_index,
            "idA": pair["idA"],
            "nameA": pair["nameA"],
            "idB": pair["idB"],
            "nameB": pair["nameB"],
        }
    return None


def _hud_slot_desc(slot_id: int) -> dict[str, object]:
    return {
        "slot_id": slot_id,
        "desc_ptr_addr": _hex32(0x801CF924 + slot_id * 12),
    }


def _summarize_selector_usage(per_flag_values: dict[str, set[int]], pair_table: list[dict[str, object]]) -> dict[str, object]:
    return {
        "flags04_0x00010000": {
            "cool_pairs": [_pair_names(pair_table, value) for value in sorted(per_flag_values["ev_14"]) if value != 0],
            "noncool_pairs": [_pair_names(pair_table, value) for value in sorted(per_flag_values["ev_15"]) if value != 0],
        },
        "flags04_0x00020000": {
            "mode0_cool_pairs": [_pair_names(pair_table, value) for value in sorted(per_flag_values["ev_10"]) if value != 0],
            "mode1_good_pairs": [_pair_names(pair_table, value) for value in sorted(per_flag_values["ev_11"]) if value != 0],
            "mode2_bad_pairs": [_pair_names(pair_table, value) for value in sorted(per_flag_values["ev_12"]) if value != 0],
            "mode3_awfl_pairs": [_pair_names(pair_table, value) for value in sorted(per_flag_values["ev_13"]) if value != 0],
        },
        "hud_slots": {
            "slot0_from_ev_17": [_hud_slot_desc(value) for value in sorted(per_flag_values["ev_17"]) if value != 0],
            "slot1_mode0_from_ev_18": [_hud_slot_desc(value) for value in sorted(per_flag_values["ev_18"]) if value != 0],
            "slot1_mode1_from_ev_19": [_hud_slot_desc(value) for value in sorted(per_flag_values["ev_19"]) if value != 0],
            "slot1_mode2_from_ev_1A": [_hud_slot_desc(value) for value in sorted(per_flag_values["ev_1A"]) if value != 0],
            "slot1_mode3_from_ev_1B": [_hud_slot_desc(value) for value in sorted(per_flag_values["ev_1B"]) if value != 0],
            "slot2_from_ev_1C": [_hud_slot_desc(value) for value in sorted(per_flag_values["ev_1C"]) if value != 0],
        },
        "text_ids": [_hex8(value) for value in sorted(per_flag_values["ev_20"]) if value != 0],
    }


def _parse_stream(
    blob: bytes,
    stream_id: int,
    top_n: int,
    handle_map: dict[int, dict[str, object]],
    pair_table: list[dict[str, object]],
    text_tables: list[dict[str, object]],
) -> dict[str, object]:
    table_addr = STREAM_TABLE_ADDR + stream_id * STREAM_ENTRY_SIZE
    events_addr = _read_u32(blob, table_addr)
    count = _read_u32(blob, table_addr + 4)
    cursor = _read_u32(blob, table_addr + 8)
    row = _read_row(blob, STREAM_DESC_ADDR + stream_id * 8, 8)

    flag_counts: Counter[int] = Counter()
    combo_counts: Counter[tuple[int, tuple[int, ...]]] = Counter()
    per_flag_column_values: dict[int, dict[str, set[int]]] = defaultdict(
        lambda: {
            "ev_10": set(),
            "ev_11": set(),
            "ev_12": set(),
            "ev_13": set(),
            "ev_14": set(),
            "ev_15": set(),
            "ev_17": set(),
            "ev_18": set(),
            "ev_19": set(),
            "ev_1A": set(),
            "ev_1B": set(),
            "ev_1C": set(),
            "ev_20": set(),
        }
    )
    samples: list[dict[str, object]] = []

    for index in range(count):
        event_addr = events_addr + index * EVENT_SIZE
        event_off = _offset(event_addr)
        frame, flags = struct.unpack_from("<II", blob, event_off)
        cols = tuple(blob[event_off + 0x10 : event_off + 0x16])
        extra = tuple(blob[event_off + 0x17 : event_off + 0x1D])
        text_id = blob[event_off + 0x20]

        flag_counts[flags] += 1
        combo_counts[(flags, cols)] += 1

        values = per_flag_column_values[flags]
        values["ev_10"].add(cols[0])
        values["ev_11"].add(cols[1])
        values["ev_12"].add(cols[2])
        values["ev_13"].add(cols[3])
        values["ev_14"].add(cols[4])
        values["ev_15"].add(cols[5])
        values["ev_17"].add(extra[0])
        values["ev_18"].add(extra[1])
        values["ev_19"].add(extra[2])
        values["ev_1A"].add(extra[3])
        values["ev_1B"].add(extra[4])
        values["ev_1C"].add(extra[5])
        values["ev_20"].add(text_id)

        if len(samples) < 8:
            samples.append(
                {
                    "event_index": index,
                    "event_addr": _hex32(event_addr),
                    "frame": frame,
                    "flags04": _hex32(flags),
                    "ev_10_15": [_hex8(value) for value in cols],
                    "ev_17_1C": [_hex8(value) for value in extra],
                    "ev_20_text_id": _hex8(text_id),
                }
            )

    selector_usage = {}
    for flags, values in sorted(per_flag_column_values.items()):
        selector_usage[_hex32(flags)] = _summarize_selector_usage(values, pair_table)

    used_text_ids = sorted({value for values in per_flag_column_values.values() for value in values["ev_20"] if value != 0})
    text_usage = []
    for table in text_tables:
        picked = []
        for text_id in used_text_ids:
            if text_id < table["entry_count"]:
                entry = table["entries"][text_id]
                picked.append(
                    {
                        "text_id": entry["text_id"],
                        "ptr": entry["ptr"],
                        "text": entry["text"],
                    }
                )
        text_usage.append(
            {
                "table_index": table["table_index"],
                "language_guess": table["language_guess"],
                "used_entries": picked,
            }
        )

    return {
        "stream_id": stream_id,
        "candidate_role_guess": ROLE_GUESSES.get(stream_id, "未命名；待动态抓取补角色/字幕/收尾语义。"),
        "stream_desc_addr": _hex32(STREAM_DESC_ADDR + stream_id * 8),
        "stream_desc": _stream_desc_summary(row, handle_map),
        "stream_entry_addr": _hex32(table_addr),
        "events_addr": _hex32(events_addr),
        "count": count,
        "cursor_initial": cursor,
        "top_flags04": [
            {"flags04": _hex32(flags), "count": hits}
            for flags, hits in flag_counts.most_common(top_n)
        ],
        "common_ev_10_15_patterns": [
            {
                "flags04": _hex32(flags),
                "ev_10_15": [_hex8(value) for value in cols],
                "count": hits,
            }
            for (flags, cols), hits in combo_counts.most_common(top_n)
        ],
        "selector_values_by_flag": {
            _hex32(flags): {
                key: [_hex8(value) for value in sorted(values)]
                for key, values in per_flag_column_values[flags].items()
            }
            for flags in sorted(per_flag_column_values)
        },
        "selector_resource_usage": selector_usage,
        "text_usage_by_table": text_usage,
        "sample_events": samples,
    }


def build_summary(blob: bytes, binary_path: Path, compo_path: Path, top_n: int) -> dict[str, object]:
    handle_map = _load_mem_handle_map(compo_path)
    pair_table = _read_pair_table(blob, handle_map)
    text_tables = _read_text_tables(blob)

    return {
        "analysis_only": True,
        "not_runtime_data_source": True,
        "binary": str(binary_path),
        "compo": str(compo_path),
        "base_addr": _hex32(BASE_ADDR),
        "stream_desc_addr": _hex32(STREAM_DESC_ADDR),
        "text_table_ptrs_addr": _hex32(TEXT_TABLE_PTRS_ADDR),
        "text_table_count": TEXT_TABLE_COUNT,
        "pair_table_addr": _hex32(PAIR_TABLE_ADDR),
        "pair_table_count": PAIR_TABLE_COUNT,
        "stream_table_addr": _hex32(STREAM_TABLE_ADDR),
        "event_size": _hex32(EVENT_SIZE),
        "selector_semantics": _selector_semantics(),
        "mem_handle_map": {
            _hex16(handle): entry for handle, entry in sorted(handle_map.items())
        },
        "text_tables": text_tables,
        "pair_table": pair_table,
        "stream_rows": {
            str(stream_id): _stream_desc_summary(
                _read_row(blob, STREAM_DESC_ADDR + stream_id * 8, 8),
                handle_map,
            )
            for stream_id in range(1, STREAM_COUNT)
        },
        "kind_rows": {
            str(row_id): _stream_desc_summary(
                _read_row(blob, STREAM_DESC_ADDR + row_id * 8, 8),
                handle_map,
            )
            for row_id in range(0, 9)
        },
        "streams": [
            _parse_stream(blob, stream_id, top_n, handle_map, pair_table, text_tables)
            for stream_id in range(1, STREAM_COUNT)
        ],
    }


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Summarize COMOD1 Stage1 event streams, selectors, and resource pairs."
    )
    parser.add_argument("--binary", type=Path, default=DEFAULT_BINARY)
    parser.add_argument("--compo", type=Path, default=DEFAULT_COMPO)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--top", type=int, default=6, help="How many top flag/pattern rows to keep.")
    return parser


def main() -> int:
    parser = _build_parser()
    args = parser.parse_args()

    blob = args.binary.read_bytes()
    summary = build_summary(blob, args.binary, args.compo, args.top)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(f"binary:   {args.binary}")
    print(f"compo:    {args.compo}")
    print(f"output:   {args.output}")
    print(f"base:     {_hex32(BASE_ADDR)}")
    print(f"textTabs: {summary['text_table_count']}")
    print(f"pairs:    {summary['pair_table_count']}")
    print(f"streams:  {len(summary['streams'])}")
    first = summary["streams"][0]
    print(
        "stream1:  events={events} count={count} desc={desc}".format(
            events=first["events_addr"],
            count=first["count"],
            desc=" ".join(first["stream_desc"]["bytes"]),
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
