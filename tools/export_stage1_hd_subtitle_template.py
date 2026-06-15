#!/usr/bin/env python3
"""Export editable Stage1 HD subtitle sidecar templates from COMOD1.BIN.

The generated TSV keeps the original subtitle/event keys and leaves the final
`text` column blank by default. Fill that column with translated text; the game
loads only rows whose final text column is non-empty.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


PSX_BASE = 0x801C3870
MOVIE1_SUBTITLE_DESC = 0x801CE080
COMMON_DESC_BY_MODE = [0x801CE080, 0x801CE09C, 0x801CE0B8]
COMMON_COUNT_BY_MODE = [41, 6, 6]
TEXT_TABLE_PTRS_ADDR = 0x801CE804
TEXT_TABLE_COUNT = 5
STREAM_TABLE_ADDR = 0x801D2D64
STREAM_COUNT = 8
STREAM_ENTRY_SIZE = 0x0C
SCRIPT_EVENT_SIZE = 0x24
LANGS = ["EN", "FR", "DE", "ES", "IT"]


@dataclass
class VTextLine:
    frame30: int
    duration: int
    text_indices: list[int]
    text_addrs: list[int]
    texts: list[str]


@dataclass
class ScriptEvent:
    stream_id: int
    psx_addr: int
    frame: int
    text_id: int
    texts: list[str]


def read_u16(data: bytes, off: int) -> int:
    return data[off] | (data[off + 1] << 8)


def read_s16(data: bytes, off: int) -> int:
    value = read_u16(data, off)
    return value - 0x10000 if value & 0x8000 else value


def read_u32(data: bytes, off: int) -> int:
    return (
        data[off]
        | (data[off + 1] << 8)
        | (data[off + 2] << 16)
        | (data[off + 3] << 24)
    )


def psx_to_off(addr: int, size: int) -> int:
    if addr < PSX_BASE:
        raise ValueError(f"PSX address 0x{addr:08X} is below base")
    off = addr - PSX_BASE
    if off < 0 or off >= size:
        raise ValueError(f"PSX address 0x{addr:08X} is outside COMOD1.BIN")
    return off


def read_c_string(data: bytes, addr: int) -> str:
    if addr == 0:
        return ""
    off = psx_to_off(addr, len(data))
    end = off
    while end < len(data) and data[end] != 0:
        end += 1
    raw = data[off:end]
    try:
        return raw.decode("ascii")
    except UnicodeDecodeError:
        return raw.decode("latin1", errors="replace")


def parse_text_tables(data: bytes) -> tuple[list[int], list[list[int]], list[list[str]]]:
    ptrs = [
        read_u32(data, psx_to_off(TEXT_TABLE_PTRS_ADDR + i * 4, len(data)))
        for i in range(TEXT_TABLE_COUNT)
    ]
    entry_ptrs: list[list[int]] = []
    texts: list[list[str]] = []
    for i, ptr in enumerate(ptrs):
        end = ptrs[i + 1] if i + 1 < len(ptrs) else TEXT_TABLE_PTRS_ADDR
        count = max(0, (end - ptr) // 4)
        table_ptrs: list[int] = []
        table_texts: list[str] = []
        for text_id in range(count):
            text_ptr = read_u32(data, psx_to_off(ptr + text_id * 4, len(data)))
            table_ptrs.append(text_ptr)
            table_texts.append(read_c_string(data, text_ptr))
        entry_ptrs.append(table_ptrs)
        texts.append(table_texts)
    return ptrs, entry_ptrs, texts


def parse_vtext_track(data: bytes, desc_addr: int, expected_count: int = 0) -> list[VTextLine]:
    desc_off = psx_to_off(desc_addr, len(data))
    text_tables = [read_u32(data, desc_off + i * 4) for i in range(5)]
    entries_addr = read_u32(data, desc_off + 20)
    entry_count = read_u32(data, desc_off + 24)
    if expected_count and entry_count != expected_count:
        raise ValueError(
            f"descriptor 0x{desc_addr:08X} count {entry_count}, expected {expected_count}"
        )

    table_offsets = [psx_to_off(addr, len(data)) for addr in text_tables]
    entries_off = psx_to_off(entries_addr, len(data))
    lines: list[VTextLine] = []
    for i in range(entry_count):
        off = entries_off + i * 16
        minute = read_u16(data, off)
        second = data[off + 2]
        frame = data[off + 3]
        duration = read_u16(data, off + 4)
        frame30 = minute * 1800 + second * 30 + frame

        text_indices: list[int] = []
        text_addrs: list[int] = []
        texts: list[str] = []
        for lang in range(5):
            idx = read_s16(data, off + 6 + lang * 2)
            text_indices.append(idx)
            text_addr = 0
            text = ""
            if idx > 0:
                ptr_pos = table_offsets[lang] + idx * 4
                if 0 <= ptr_pos <= len(data) - 4:
                    text_addr = read_u32(data, ptr_pos)
                    text = read_c_string(data, text_addr)
            text_addrs.append(text_addr)
            texts.append(text)

        lines.append(VTextLine(frame30, duration, text_indices, text_addrs, texts))
    return lines


def parse_script_events(data: bytes, text_tables: list[list[str]]) -> list[ScriptEvent]:
    events: list[ScriptEvent] = []
    for stream_id in range(1, STREAM_COUNT + 1):
        desc_addr = STREAM_TABLE_ADDR + stream_id * STREAM_ENTRY_SIZE
        desc_off = psx_to_off(desc_addr, len(data))
        events_ptr = read_u32(data, desc_off)
        count = read_u32(data, desc_off + 4)
        if events_ptr == 0 or count == 0:
            continue
        for i in range(count):
            ev_addr = events_ptr + i * SCRIPT_EVENT_SIZE
            ev_off = psx_to_off(ev_addr, len(data))
            frame = read_u32(data, ev_off)
            text_id = data[ev_off + 0x20]
            if text_id == 0:
                continue
            texts = [
                table[text_id] if text_id < len(table) else ""
                for table in text_tables
            ]
            if not any(texts):
                continue
            events.append(ScriptEvent(stream_id, ev_addr, frame, text_id, texts))
    return events


def escape_tsv(text: str) -> str:
    return (
        text.replace("\\", "\\\\")
        .replace("\t", " ")
        .replace("\r", "")
        .replace("\n", "\\n")
    )


def choose_source_text(texts: list[str], source_lang: str) -> tuple[str, str]:
    source_lang = source_lang.upper()
    candidates = []
    if source_lang in LANGS:
        candidates.append(LANGS.index(source_lang))
    candidates.extend(range(len(LANGS)))
    seen = set()
    for idx in candidates:
        if idx in seen:
            continue
        seen.add(idx)
        if 0 <= idx < len(texts) and texts[idx]:
            return LANGS[idx], texts[idx]
    return source_lang, ""


def make_row(
    language: str,
    scope: str,
    mode: str | int,
    frame30: str | int,
    duration: str | int,
    text_index: str | int,
    text_id: str | int,
    psx_addr: str | int,
    original_language: str,
    original_text: str,
    text: str,
) -> str:
    def fmt(value: str | int) -> str:
        if isinstance(value, int):
            return str(value)
        return value

    if isinstance(psx_addr, int):
        psx_addr_s = f"0x{psx_addr:08X}" if psx_addr else "*"
    else:
        psx_addr_s = psx_addr
    fields = [
        language,
        scope,
        fmt(mode),
        fmt(frame30),
        fmt(duration),
        fmt(text_index),
        fmt(text_id),
        psx_addr_s,
        original_language,
        escape_tsv(original_text),
        escape_tsv(text),
    ]
    return "\t".join(fields)


def iter_template_rows(
    data: bytes,
    target_language: str,
    source_language: str,
    copy_source_text: bool,
    include_original_language_rows: bool,
) -> Iterable[str]:
    _ptrs, _entry_ptrs, text_tables = parse_text_tables(data)

    def emit_target_for_vtext(scope: str, mode: int, line: VTextLine) -> str | None:
        orig_lang, orig_text = choose_source_text(line.texts, source_language)
        if not orig_text:
            return None
        lang_index = LANGS.index(orig_lang)
        text = orig_text if copy_source_text else ""
        return make_row(
            target_language,
            scope,
            mode,
            line.frame30,
            line.duration,
            line.text_indices[lang_index],
            "*",
            line.text_addrs[lang_index],
            orig_lang,
            orig_text,
            text,
        )

    def emit_original_for_vtext(scope: str, mode: int, line: VTextLine) -> Iterable[str]:
        for lang_index, lang in enumerate(LANGS):
            text = line.texts[lang_index]
            if not text:
                continue
            yield make_row(
                lang,
                scope,
                mode,
                line.frame30,
                line.duration,
                line.text_indices[lang_index],
                "*",
                line.text_addrs[lang_index],
                lang,
                text,
                text,
            )

    movie_lines = parse_vtext_track(data, MOVIE1_SUBTITLE_DESC, 41)
    for line in movie_lines:
        row = emit_target_for_vtext("movie1", 0, line)
        if row:
            yield row
        if include_original_language_rows:
            yield from emit_original_for_vtext("movie1", 0, line)

    for mode, desc_addr in enumerate(COMMON_DESC_BY_MODE):
        lines = parse_vtext_track(data, desc_addr, COMMON_COUNT_BY_MODE[mode])
        for line in lines:
            row = emit_target_for_vtext("common", mode, line)
            if row:
                yield row
            if include_original_language_rows:
                yield from emit_original_for_vtext("common", mode, line)

    script_events = parse_script_events(data, text_tables)
    for ev in script_events:
        orig_lang, orig_text = choose_source_text(ev.texts, source_language)
        if not orig_text:
            continue
        text = orig_text if copy_source_text else ""
        yield make_row(
            target_language,
            "script",
            "*",
            ev.frame,
            120,
            "*",
            ev.text_id,
            ev.psx_addr,
            orig_lang,
            orig_text,
            text,
        )
        if include_original_language_rows:
            for lang_index, lang in enumerate(LANGS):
                source = ev.texts[lang_index]
                if not source:
                    continue
                yield make_row(
                    lang,
                    "script",
                    "*",
                    ev.frame,
                    120,
                    "*",
                    ev.text_id,
                    ev.psx_addr,
                    lang,
                    source,
                    source,
                )


def find_default_comod(data_root: Path) -> Path:
    candidates = [
        data_root / "S1" / "COMOD1.BIN",
        data_root / "win" / "S1" / "COMOD1.BIN",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--data-root", type=Path, default=Path(".."))
    parser.add_argument("--comod", type=Path)
    parser.add_argument("--out", type=Path, default=Path("ex/subtitles/stage1_hd_zh.tsv"))
    parser.add_argument("--target-language", default="CN")
    parser.add_argument("--source-language", default="EN")
    parser.add_argument("--copy-source-text", action="store_true")
    parser.add_argument("--include-original-language-rows", action="store_true")
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    comod = args.comod or find_default_comod(args.data_root)
    if not comod.exists():
        raise SystemExit(f"COMOD1.BIN not found: {comod}")
    if args.out.exists() and not args.force:
        raise SystemExit(f"output exists, pass --force to overwrite: {args.out}")

    data = comod.read_bytes()
    rows = list(
        iter_template_rows(
            data,
            args.target_language.upper(),
            args.source_language.upper(),
            args.copy_source_text,
            args.include_original_language_rows,
        )
    )

    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# Stage1 external HD subtitles exported from COMOD1.BIN.\n")
        f.write("# Fill the final text column for translated rows. Blank text rows are ignored by the runtime.\n")
        f.write(
            "language\tscope\tmode\tframe30\tduration\ttext_index\t"
            "text_id\tpsx_addr\toriginal_language\toriginal_text\ttext\n"
        )
        for row in rows:
            f.write(row)
            f.write("\n")

    print(f"wrote {len(rows)} rows to {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
