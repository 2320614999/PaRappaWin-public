#!/usr/bin/env python3
"""Stage1 rail full-flow capture v2 for live/manual Stage1 sessions."""

from __future__ import annotations

import argparse
from collections import Counter
import datetime as _dt
import json
import re
import signal
import time
from pathlib import Path
from typing import Any

from debug_client import DebugClient, HOST as DEBUG_HOST, PORT as DEBUG_PORT
import stage1_dual_path_capture as dual


REPO_ROOT = Path(__file__).resolve().parent
DEFAULT_OUTPUT_DIR = REPO_ROOT / "artifacts" / "stage1_rail_capture_v2"

MAIN_EVENT_TICK_BASE = 0x0008EEF8
MAIN_EVENT_MASK_BASE = 0x0008EEFC

ICON_SLOT_COUNT = 18
ICON_X0 = 41
ICON_X_STEP = 14
ICON_Y = 99
PORTRAIT_BASE_X = 16
PORTRAIT_Y = 93
PORTRAIT_CURSOR_STEP_X = 15
PORTRAIT_FRAME_STEP_X = 4
PORTRAIT_MAX_FRAME = 3
REC44_TOTAL_SIZE = 44
REC44_STREAM_SIZE = 40

ICON_ASSET_NAMES = {
    1: "GUI_SANK",
    2: "GUI_MARU",
    3: "GUI_PEKE",
    4: "GUI_SIKA",
    5: "GUI_L",
    6: "GUI_L",
    7: "GUI_R",
    8: "GUI_R",
}

STOP_REQUESTED = False


def _signal_handler(signum: int, frame: Any) -> None:
    del signum, frame
    global STOP_REQUESTED
    STOP_REQUESTED = True


signal.signal(signal.SIGINT, _signal_handler)
signal.signal(signal.SIGTERM, _signal_handler)


def _u32(buf: bytes, off: int) -> int:
    return int.from_bytes(buf[off:off + 4], "little", signed=False)


def _u16(buf: bytes, off: int) -> int:
    return int.from_bytes(buf[off:off + 2], "little", signed=False)


def _s16(buf: bytes, off: int) -> int:
    return int.from_bytes(buf[off:off + 2], "little", signed=True)


def _s8(buf: bytes, off: int) -> int:
    return int.from_bytes(buf[off:off + 1], "little", signed=True)


def _psx_to_ram(addr: int) -> int:
    return addr & 0x1FFFFFFF


def _safe_reader_read(reader: dual.GdbRunningReader, address: int, length: int) -> bytes:
    try:
        return reader.read_memory(address, length)
    except Exception:
        return b""


def _read_main_event(reader: dual.GdbRunningReader, index: int) -> dict[str, int] | None:
    if index < 0:
        return None
    tick = _safe_reader_read(reader, MAIN_EVENT_TICK_BASE + index * 8, 4)
    mask = _safe_reader_read(reader, MAIN_EVENT_MASK_BASE + index * 8, 4)
    if len(tick) != 4 or len(mask) != 4:
        return None
    return {
        "index": index,
        "tick": _u32(tick, 0),
        "mask": _u32(mask, 0),
    }


def _read_rec44(reader: dual.GdbRunningReader, ptr: int) -> dict[str, Any] | None:
    if ptr == 0:
        return None
    ram = _psx_to_ram(ptr)
    if ram < 4:
        return None
    blob = _safe_reader_read(reader, ram - 4, REC44_TOTAL_SIZE)
    if len(blob) != REC44_TOTAL_SIZE:
        return None
    stream = [_s8(blob, 4 + i) for i in range(REC44_STREAM_SIZE)]
    return {
        "ptr": ptr,
        "ram": ram,
        "head0": _s16(blob, 0),
        "head1": _u16(blob, 2),
        "stream": stream,
        "stream18": stream[:ICON_SLOT_COUNT],
    }


def _stream18(stream: dict[str, Any] | None) -> list[int] | None:
    if not stream:
        return None
    values = stream.get("stream18")
    if values is None:
        return None
    return [int(v) for v in values]


def _read_stage1_rail_raw(reader: dual.GdbRunningReader) -> dict[str, Any]:
    ctx = _safe_reader_read(reader, dual.CTX_BASE, dual.CTX_SIZE)
    if len(ctx) != dual.CTX_SIZE:
        return {}

    ptr94 = _u32(ctx, 0x94)
    ptr98 = _u32(ctx, 0x98)
    ptrA4 = _u32(ctx, 0xA4)
    ptrA8 = _u32(ctx, 0xA8)
    return {
        "row_mode": _s16(ctx, 0x8A),
        "teacher_cursor": _s16(ctx, 0x8C),
        "teacher_aux_cursor": _s16(ctx, 0x8E),
        "teacher_enable": _u16(ctx, 0x90),
        "stream94_ptr": ptr94,
        "stream98_ptr": ptr98,
        "student_cursor": _s16(ctx, 0x9E),
        "student_aux_cursor": _s16(ctx, 0xA0),
        "student_enable": _s16(ctx, 0xA2),
        "streamA4_ptr": ptrA4,
        "streamA8_ptr": ptrA8,
        "stream94": _read_rec44(reader, ptr94),
        "stream98": _read_rec44(reader, ptr98),
        "streamA4": _read_rec44(reader, ptrA4),
        "streamA8": _read_rec44(reader, ptrA8),
    }


def _make_icon_slot_record(slot_index: int, code: int) -> dict[str, Any]:
    return {
        "slot": slot_index,
        "code": code,
        "asset": ICON_ASSET_NAMES.get(code),
        "visible": code in ICON_ASSET_NAMES,
        "x": ICON_X0 + ICON_X_STEP * slot_index,
        "y": ICON_Y,
    }


def _derive_portrait_asset(actor: str) -> str | None:
    if actor == "teacher":
        return "KT_SI"
    if actor == "student":
        return "PA_SI"
    return None


def _update_portrait_anim_state(
    actor_state: dict[str, Any],
    tick96: int,
    visible: bool,
    cursor: int,
) -> int:
    if actor_state.get("last_tick96") == tick96:
        return int(actor_state.get("frame", 0))

    if not visible or cursor < 0:
        frame = 0
    elif actor_state.get("last_cursor") == cursor:
        frame = min(int(actor_state.get("frame", 0)) + 1, PORTRAIT_MAX_FRAME)
    else:
        frame = 0

    actor_state["last_tick96"] = tick96
    actor_state["last_cursor"] = cursor
    actor_state["frame"] = frame
    return frame


def _derive_portrait_state(
    actor: str,
    tick96: int,
    cursor: int,
    enable: int,
    anim_state: dict[str, Any],
) -> dict[str, Any]:
    visible = cursor >= 0 and enable != 0
    frame = _update_portrait_anim_state(anim_state, tick96, visible, cursor)
    x = None
    if visible and cursor >= 0:
        x = PORTRAIT_BASE_X + PORTRAIT_CURSOR_STEP_X * cursor + PORTRAIT_FRAME_STEP_X * frame
    return {
        "cursor": cursor,
        "enable": enable,
        "visible": visible,
        "frame": frame,
        "x": x,
        "y": PORTRAIT_Y if visible else None,
        "asset_family_inferred": _derive_portrait_asset(actor),
        "asset_family_observed": False,
    }


def _derive_display_mode(
    icon_slots: list[dict[str, Any]],
    teacher: dict[str, Any],
    student: dict[str, Any],
) -> str:
    icons_visible = any(slot["visible"] for slot in icon_slots)
    portraits_visible = bool(teacher["visible"] or student["visible"])
    if icons_visible and portraits_visible:
        return "icons+portraits"
    if icons_visible:
        return "icons-only"
    if portraits_visible:
        return "portraits-only"
    return "hidden"


def _derive_payloads(
    base_snapshot: dict[str, Any],
    main_event_current: dict[str, int] | None,
    main_event_next: dict[str, int] | None,
) -> dict[str, Any]:
    overlay = base_snapshot["overlay"]
    late = base_snapshot["late_branch"]
    acc = base_snapshot["accepted_formula"]
    return {
        "current": {
            "kind": "overlay_active_event",
            "available": overlay["resolved_event"] is not None,
            "event": overlay["resolved_event"],
            "timing": main_event_current,
            "inference": "current live overlay event / HUD slot source",
        },
        "accepted": {
            "kind": "accepted_formula_aggregate",
            "available": True,
            "accepted_mask_accum": acc["accepted_mask_accum"],
            "accepted_contribution_count": acc["accepted_contribution_count"],
            "recorded_split_count": acc["recorded_split_count"],
            "penalty_split_count": acc["penalty_split_count"],
            "score_accumulator": acc["score_accumulator"],
            "good_to_cool_gate_enabled": acc["good_to_cool_gate_enabled"],
            "inference": "accepted-boundary aggregate only; no pure-emulator symbol split",
        },
        "next": {
            "kind": "late_branch_row_source",
            "available": True,
            "selected_stream": late["selected_stream"],
            "selected_row": late["selected_row"],
            "selected_row_handles": late["selected_row_handles"],
            "selected_row_names": late["selected_row_names"],
            "selected_row_ptrs": late["selected_row_ptrs"],
            "consumer_row_handles": late["consumer_row_handles"],
            "consumer_row_ptrs": late["consumer_row_ptrs"],
            "timing": main_event_next,
            "inference": "next row/data-source candidate from late-branch runtime + main event cursor",
        },
    }


def _voice_signature(voice_lane: dict[str, Any]) -> tuple[int, int, int, int, int]:
    return (
        int(voice_lane["replace_restart_handle"]),
        int(voice_lane["program"]),
        int(voice_lane["note"]),
        int(voice_lane["key"]),
        int(voice_lane["volume"]),
    )


def _build_rail_sfx_state(
    base_snapshot: dict[str, Any],
    main_event_current: dict[str, int] | None,
) -> dict[str, Any]:
    ctx = base_snapshot["ctx"]
    late = base_snapshot["late_branch"]
    voice = base_snapshot["voice_lane"]
    acc = base_snapshot["accepted_formula"]
    return {
        "dispatch_available": any(_voice_signature(voice)),
        "cue_kind": "voice_lane_raw",
        "trigger": {
            "tick96": ctx["tick96"],
            "bucket": ctx["current_bucket"],
            "event": main_event_current,
        },
        "bucket": ctx["current_bucket"],
        "follow_up_phase_coarse": late["follow_up_phase_coarse"],
        "source_link": {
            "selected_stream": late["selected_stream"],
            "selected_row": late["selected_row"],
            "selected_row_handles": late["selected_row_handles"],
            "accepted_contribution_count": acc["accepted_contribution_count"],
            "recorded_split_count": acc["recorded_split_count"],
            "penalty_split_count": acc["penalty_split_count"],
            "good_to_cool_gate_enabled": acc["good_to_cool_gate_enabled"],
        },
        "voice_lane": voice,
    }


def _derive_anomaly_candidates(
    icon_slots: list[dict[str, Any]],
    teacher: dict[str, Any],
    student: dict[str, Any],
    raw: dict[str, Any],
) -> dict[str, Any]:
    unknown_codes = [
        slot["code"]
        for slot in icon_slots
        if slot["code"] not in (-1, 0) and slot["code"] not in ICON_ASSET_NAMES
    ]
    return {
        "fake_note_pollution": bool(unknown_codes),
        "fake_note_codes": unknown_codes,
        "wrong_portrait_candidate": False,
        "wrong_y_candidate": False,
        "intermittent_hide_candidate": False,
        "notes": [
            "capture v2 auto-flags unknown icon codes only",
            "portrait family and Y mismatches still need visual correlation",
        ],
        "raw_enable": {
            "teacher_enable": raw.get("teacher_enable"),
            "student_enable": raw.get("student_enable"),
            "teacher_cursor": teacher["cursor"],
            "student_cursor": student["cursor"],
        },
    }


def _main_event_signature(event: dict[str, Any] | None) -> tuple[Any, Any, Any]:
    if not event:
        return (None, None, None)
    return (event.get("index"), event.get("tick"), event.get("mask"))


def _build_record(
    sample_index: int,
    elapsed_ms: int,
    base_snapshot: dict[str, Any],
    raw: dict[str, Any],
    main_event_current: dict[str, int] | None,
    main_event_next: dict[str, int] | None,
    portrait_anim: dict[str, dict[str, Any]],
    prev_record: dict[str, Any] | None,
) -> dict[str, Any]:
    ctx = base_snapshot["ctx"]
    overlay = base_snapshot["overlay"]
    icon_codes = list((_stream18(raw.get("stream94")) or [])[:ICON_SLOT_COUNT])
    if len(icon_codes) < ICON_SLOT_COUNT:
        icon_codes.extend([0] * (ICON_SLOT_COUNT - len(icon_codes)))
    icon_slots = [_make_icon_slot_record(i, code) for i, code in enumerate(icon_codes)]
    teacher = _derive_portrait_state(
        "teacher",
        ctx["tick96"],
        int(raw.get("teacher_cursor", -1)),
        int(raw.get("teacher_enable", 0)),
        portrait_anim["teacher"],
    )
    student = _derive_portrait_state(
        "student",
        ctx["tick96"],
        int(raw.get("student_cursor", -1)),
        int(raw.get("student_enable", 0)),
        portrait_anim["student"],
    )
    display_mode = _derive_display_mode(icon_slots, teacher, student)
    rail_visible = display_mode != "hidden"
    payloads = _derive_payloads(base_snapshot, main_event_current, main_event_next)
    rail_sfx = _build_rail_sfx_state(base_snapshot, main_event_current)
    anomaly_candidates = _derive_anomaly_candidates(icon_slots, teacher, student, raw)

    reasons: list[str] = []
    if prev_record is None:
        reasons.append("capture_start")
    else:
        prev_display = prev_record["rail_display"]
        if rail_visible != prev_display["visible"]:
            reasons.append("visibility_change")
        if display_mode != prev_display["display_mode"]:
            reasons.append("display_mode_change")
        if tuple(icon_codes) != tuple(prev_display["icon_codes"]):
            reasons.append("icon_stream_change")
        if (
            teacher["cursor"] != prev_display["teacher_portrait"]["cursor"]
            or teacher["visible"] != prev_display["teacher_portrait"]["visible"]
        ):
            reasons.append("teacher_cursor_change")
        if (
            student["cursor"] != prev_display["student_portrait"]["cursor"]
            or student["visible"] != prev_display["student_portrait"]["visible"]
        ):
            reasons.append("student_cursor_change")
        if (
            _stream18(raw.get("streamA4"))
            != prev_record["rail_data_source"]["body_stream_primary_stream18"]
        ):
            reasons.append("body_stream_change")
        if (
            _stream18(raw.get("streamA8"))
            != prev_record["rail_data_source"]["body_stream_alt_stream18"]
        ):
            reasons.append("body_stream_alt_change")
        if raw.get("row_mode") != prev_record["rail_data_source"]["row_mode"]:
            reasons.append("row_mode_change")
        if (
            base_snapshot["late_branch"]["selected_row"]
            != prev_record["rail_data_source"]["late_branch"]["selected_row"]
        ):
            reasons.append("late_branch_row_change")
        if _main_event_signature(main_event_current) != _main_event_signature(
            ((prev_record.get("rail_payloads") or {}).get("current") or {}).get("timing")
        ):
            reasons.append("main_event_current_change")
        if _main_event_signature(main_event_next) != _main_event_signature(
            ((prev_record.get("rail_payloads") or {}).get("next") or {}).get("timing")
        ):
            reasons.append("main_event_next_change")
        if _voice_signature(base_snapshot["voice_lane"]) != _voice_signature(prev_record["rail_sfx"]["voice_lane"]):
            reasons.append("rail_sfx_change")
        if base_snapshot["ctx"]["current_bucket"] != prev_record["timing"]["bucket"]:
            reasons.append("bucket_change")

    return {
        "api": "openai-responses",
        "sample_index": sample_index,
        "elapsed_ms": elapsed_ms,
        "timing": {
            "tick96": ctx["tick96"],
            "bucket": ctx["current_bucket"],
            "main_event_cursor": overlay["main_event_cursor"],
            "main_event_count": overlay["main_event_count"],
            "overlay_event_frame": overlay["resolved_event"]["frame"] if overlay["resolved_event"] else None,
            "overlay_stream_id": overlay["active_stream_id"],
        },
        "reasons": reasons,
        "rail_display": {
            "visible": rail_visible,
            "display_mode": display_mode,
            "draw_path": "doc-derived PSX helper path (sub_80024418 + sub_80024600)",
            "icon_codes": icon_codes,
            "icons": icon_slots,
            "teacher_portrait": teacher,
            "student_portrait": student,
            "icon_layout": {
                "x0": ICON_X0,
                "step_x": ICON_X_STEP,
                "y": ICON_Y,
                "count": ICON_SLOT_COUNT,
            },
            "portrait_layout": {
                "base_x": PORTRAIT_BASE_X,
                "y": PORTRAIT_Y,
                "cursor_step_x": PORTRAIT_CURSOR_STEP_X,
                "frame_step_x": PORTRAIT_FRAME_STEP_X,
                "max_frame": PORTRAIT_MAX_FRAME,
            },
            "asset_family": {
                "icons": "GUI_*",
                "teacher_portrait": teacher["asset_family_inferred"],
                "student_portrait": student["asset_family_inferred"],
                "teacher_alt_known": "ON_SI",
                "student_alt_known": None,
            },
        },
        "rail_payloads": payloads,
        "rail_data_source": {
            "row_mode": raw.get("row_mode"),
            "teacher_enable": raw.get("teacher_enable"),
            "student_enable": raw.get("student_enable"),
            "teacher_cursor": raw.get("teacher_cursor"),
            "teacher_aux_cursor": raw.get("teacher_aux_cursor"),
            "student_cursor": raw.get("student_cursor"),
            "student_aux_cursor": raw.get("student_aux_cursor"),
            "icon_stream_primary_stream18": _stream18(raw.get("stream94")),
            "icon_stream_alt_stream18": _stream18(raw.get("stream98")),
            "body_stream_primary_stream18": _stream18(raw.get("streamA4")),
            "body_stream_alt_stream18": _stream18(raw.get("streamA8")),
            "stream94": raw.get("stream94"),
            "stream98": raw.get("stream98"),
            "streamA4": raw.get("streamA4"),
            "streamA8": raw.get("streamA8"),
            "late_branch": base_snapshot["late_branch"],
            "overlay": base_snapshot["overlay"],
            "accepted_formula": base_snapshot["accepted_formula"],
        },
        "rail_sfx": rail_sfx,
        "anomaly_candidates": anomaly_candidates,
    }


def _search_int(src: str, pattern: str) -> int | None:
    match = re.search(pattern, src)
    if not match:
        return None
    return int(match.group(1))


def _search_float(src: str, pattern: str) -> float | None:
    match = re.search(pattern, src)
    if not match:
        return None
    return float(match.group(1))


def _search_int_list(src: str, pattern: str) -> list[int] | None:
    match = re.search(pattern, src, re.S)
    if not match:
        return None
    blob = (match.group(1) or "").strip()
    if not blob:
        return []
    out: list[int] = []
    for part in blob.split(","):
        token = part.strip()
        if not token:
            continue
        out.append(int(token))
    return out


def _int_to_bool(value: int | None) -> bool | None:
    if value is None:
        return None
    return bool(value)


def _parse_rail_cue(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "symbolType": _search_int(src, rf"{re.escape(name)}\.symbolType=(-?\d+)"),
    }


def _parse_rail_actor(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "enable": _search_int(src, rf"{re.escape(name)}\.enable=(-?\d+)"),
        "portraitVisible": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.portraitVisible=(-?\d+)")),
        "effectiveCursor": _search_int(src, rf"{re.escape(name)}\.effectiveCursor=(-?\d+)"),
        "rollFrame": _search_int(src, rf"{re.escape(name)}\.rollFrame=(-?\d+)"),
        "portraitCenterXValid": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.portraitCenterXValid=(-?\d+)")
        ),
        "portraitCenterX": _search_float(
            src,
            rf"{re.escape(name)}\.portraitCenterX=(-?\d+(?:\.\d+)?)",
        ),
    }


def parse_stage1ovl_rail(raw: str) -> dict[str, Any]:
    src = (raw or "").strip()
    result = {
        "command_ok": bool(src) and not src.startswith("ERROR:"),
        "runtime_available": False,
        "parse_ok": False,
        "error": None,
        "raw": src,
        "rail": {
            "valid": None,
            "queryFrame": None,
            "rowMode": None,
            "bodyStreamA": None,
            "currentCommittedCue": {"available": None, "symbolType": None},
            "nextLookaheadCue": {"available": None, "symbolType": None},
            "nextLookaheadCueWindowActive": None,
            "nextLookaheadAcceptedContributionEnabled": None,
            "acceptedBoundaryCue": {"available": None, "symbolType": None},
            "acceptedBoundarySourceCellGateActive": None,
            "acceptedBoundaryMaterialized": None,
            "acceptedHandoffPhase": {"name": None, "value": None},
        },
        "cursor": {
            "valid": None,
            "queryFrame": None,
            "rowMode": None,
            "teacher": _parse_rail_actor("", "teacher"),
            "student": _parse_rail_actor("", "student"),
        },
        "displayGate": {
            "valid": None,
            "queryFrame": None,
            "lessonSelector": None,
            "lessonVisible": None,
            "railVisibility": {
                "narrowVisibleCandidate": None,
                "cueWindowGateActive": None,
                "sourceCellGateActive": None,
                "materializedGateActive": None,
            },
        },
    }

    if not src:
        result["error"] = "empty_response"
        return result
    if src.startswith("ERROR:") or src.startswith("ERR"):
        result["error"] = src
        return result
    if "runtime=<none>" in src:
        result["runtime_available"] = False
        result["parse_ok"] = True
        result["error"] = "runtime_unavailable"
        return result

    rail = result["rail"]
    cursor = result["cursor"]
    display_gate = result["displayGate"]
    handoff = re.search(r"rail\.acceptedHandoffPhase=([A-Za-z]+)\((-?\d+)\)", src)
    rail.update(
        {
            "valid": _int_to_bool(_search_int(src, r"rail\.valid=(-?\d+)")),
            "queryFrame": _search_int(src, r"rail\.queryFrame=(-?\d+)"),
            "rowMode": _search_int(src, r"rail\.rowMode=(-?\d+)"),
            "bodyStreamA": _search_int_list(src, r"rail\.bodyStreamA\[18\]=\[(.*?)\]"),
            "currentCommittedCue": _parse_rail_cue(src, "currentCommittedCue"),
            "nextLookaheadCue": _parse_rail_cue(src, "nextLookaheadCue"),
            "nextLookaheadCueWindowActive": _int_to_bool(
                _search_int(src, r"rail\.nextLookaheadCueWindowActive=(-?\d+)")
            ),
            "nextLookaheadAcceptedContributionEnabled": _int_to_bool(
                _search_int(src, r"rail\.nextLookaheadAcceptedContributionEnabled=(-?\d+)")
            ),
            "acceptedBoundaryCue": _parse_rail_cue(src, "acceptedBoundaryCue"),
            "acceptedBoundarySourceCellGateActive": _int_to_bool(
                _search_int(src, r"rail\.acceptedBoundarySourceCellGateActive=(-?\d+)")
            ),
            "acceptedBoundaryMaterialized": _int_to_bool(
                _search_int(src, r"rail\.acceptedBoundaryMaterialized=(-?\d+)")
            ),
            "acceptedHandoffPhase": {
                "name": handoff.group(1) if handoff else None,
                "value": int(handoff.group(2)) if handoff else None,
            },
        }
    )
    cursor.update(
        {
            "valid": _int_to_bool(_search_int(src, r"cursor\.valid=(-?\d+)")),
            "queryFrame": _search_int(src, r"cursor\.queryFrame=(-?\d+)"),
            "rowMode": _search_int(src, r"cursor\.rowMode=(-?\d+)"),
            "teacher": _parse_rail_actor(src, "teacher"),
            "student": _parse_rail_actor(src, "student"),
        }
    )
    display_gate.update(
        {
            "valid": _int_to_bool(_search_int(src, r"displayGate\.valid=(-?\d+)")),
            "queryFrame": _search_int(src, r"displayGate\.queryFrame=(-?\d+)"),
            "lessonSelector": _search_int(src, r"displayGate\.lessonSelector=(-?\d+)"),
            "lessonVisible": _int_to_bool(_search_int(src, r"displayGate\.lessonVisible=(-?\d+)")),
            "railVisibility": {
                "narrowVisibleCandidate": _int_to_bool(
                    _search_int(src, r"displayGate\.railVisibility\.narrowVisibleCandidate=(-?\d+)")
                ),
                "cueWindowGateActive": _int_to_bool(
                    _search_int(src, r"displayGate\.railVisibility\.cueWindowGateActive=(-?\d+)")
                ),
                "sourceCellGateActive": _int_to_bool(
                    _search_int(src, r"displayGate\.railVisibility\.sourceCellGateActive=(-?\d+)")
                ),
                "materializedGateActive": _int_to_bool(
                    _search_int(src, r"displayGate\.railVisibility\.materializedGateActive=(-?\d+)")
                ),
            },
        }
    )

    result["runtime_available"] = True
    essential = (
        rail["queryFrame"],
        rail["rowMode"],
        rail["bodyStreamA"],
        cursor["queryFrame"],
        display_gate["queryFrame"],
    )
    result["parse_ok"] = all(value is not None for value in essential)
    if not result["parse_ok"]:
        result["error"] = "parse_incomplete"
    return result


def _extract_status_subset(status: dict[str, Any]) -> dict[str, Any]:
    if not isinstance(status, dict):
        return {"error": "status_not_dict"}
    if status.get("error"):
        return {"error": status.get("error")}
    fields = (
        "scene",
        "stageRunning",
        "stageFrame",
        "stage1OvlValid",
        "stage1OvlActiveStream",
        "stage1OvlActiveQueryFrame",
    )
    return {name: status.get(name) for name in fields}


def _timeline_win_entry(parsed: dict[str, Any], status: dict[str, Any]) -> dict[str, Any]:
    entry = {
        "command_ok": bool(parsed.get("command_ok")),
        "runtime_available": bool(parsed.get("runtime_available")),
        "parse_ok": bool(parsed.get("parse_ok")),
        "error": parsed.get("error"),
        "status": status,
        "rail": parsed.get("rail"),
        "cursor": parsed.get("cursor"),
        "displayGate": parsed.get("displayGate"),
    }
    if not entry["parse_ok"] and parsed.get("raw"):
        entry["raw_excerpt"] = parsed["raw"][:512]
    return entry


def query_win_stage1_rail(
    client: DebugClient,
    *,
    include_status: bool = True,
) -> dict[str, Any]:
    rail_start = time.monotonic()
    raw = client.send_full("stage1ovl rail")
    rail_query_ms = int((time.monotonic() - rail_start) * 1000.0)

    status: dict[str, Any] = {}
    status_query_ms = 0
    if include_status:
        status_start = time.monotonic()
        status = _extract_status_subset(client.json_status())
        status_query_ms = int((time.monotonic() - status_start) * 1000.0)

    parsed = parse_stage1ovl_rail(raw)
    return {
        "raw": raw,
        "parsed": parsed,
        "status": status,
        "timeline": _timeline_win_entry(parsed, status),
        "timing": {
            "rail_query_ms": rail_query_ms,
            "status_query_ms": status_query_ms,
        },
    }


def _compare_values(left: Any, right: Any) -> bool | None:
    if left is None or right is None:
        return None
    return left == right


def _compare_lists(left: list[int] | None, right: list[int] | None) -> bool | None:
    if left is None or right is None:
        return None
    return list(left) == list(right)


def build_dual_comparison(psx_record: dict[str, Any], win_timeline: dict[str, Any]) -> dict[str, Any]:
    win_rail = win_timeline.get("rail") or {}
    win_cursor = win_timeline.get("cursor") or {}
    win_gate = win_timeline.get("displayGate") or {}
    win_visibility = win_gate.get("railVisibility") or {}
    teacher_psx = psx_record["rail_display"]["teacher_portrait"]
    student_psx = psx_record["rail_display"]["student_portrait"]
    teacher_win = win_cursor.get("teacher") or {}
    student_win = win_cursor.get("student") or {}
    psx_body = psx_record["rail_data_source"]["body_stream_primary_stream18"]
    win_body = win_rail.get("bodyStreamA")
    return {
        "timeline_bridge": {
            "psx_tick96": psx_record["timing"]["tick96"],
            "psx_bucket": psx_record["timing"]["bucket"],
            "psx_main_event_cursor": psx_record["timing"]["main_event_cursor"],
            "psx_overlay_event_frame": psx_record["timing"]["overlay_event_frame"],
            "win_queryFrame": win_rail.get("queryFrame"),
            "win_cursorQueryFrame": win_cursor.get("queryFrame"),
            "win_displayGateQueryFrame": win_gate.get("queryFrame"),
            "win_stageFrame": (win_timeline.get("status") or {}).get("stageFrame"),
        },
        "ahead_of_time_body": {
            "psx_body_stream18": psx_body,
            "win_bodyStreamA": win_body,
            "exact_match": _compare_lists(psx_body, win_body),
        },
        "on_beat_cues": {
            "currentCommittedCue": win_rail.get("currentCommittedCue"),
            "nextLookaheadCue": win_rail.get("nextLookaheadCue"),
            "nextLookaheadCueWindowActive": win_rail.get("nextLookaheadCueWindowActive"),
            "nextLookaheadAcceptedContributionEnabled": win_rail.get(
                "nextLookaheadAcceptedContributionEnabled"
            ),
            "acceptedBoundaryCue": win_rail.get("acceptedBoundaryCue"),
            "acceptedBoundarySourceCellGateActive": win_rail.get(
                "acceptedBoundarySourceCellGateActive"
            ),
            "acceptedBoundaryMaterialized": win_rail.get("acceptedBoundaryMaterialized"),
            "acceptedHandoffPhase": win_rail.get("acceptedHandoffPhase"),
        },
        "portrait_handoff": {
            "teacher": {
                "psx": teacher_psx,
                "win": teacher_win,
                "cursor_match": _compare_values(
                    teacher_psx.get("cursor"),
                    teacher_win.get("effectiveCursor"),
                ),
                "visibility_match": _compare_values(
                    teacher_psx.get("visible"),
                    teacher_win.get("portraitVisible"),
                ),
            },
            "student": {
                "psx": student_psx,
                "win": student_win,
                "cursor_match": _compare_values(
                    student_psx.get("cursor"),
                    student_win.get("effectiveCursor"),
                ),
                "visibility_match": _compare_values(
                    student_psx.get("visible"),
                    student_win.get("portraitVisible"),
                ),
            },
        },
        "visibility_window": {
            "psx_derived_visible": psx_record["rail_display"]["visible"],
            "psx_display_mode": psx_record["rail_display"]["display_mode"],
            "win_lessonSelector": win_gate.get("lessonSelector"),
            "win_lessonVisible": win_gate.get("lessonVisible"),
            "win_narrowVisibleCandidate": win_visibility.get("narrowVisibleCandidate"),
            "win_cueWindowGateActive": win_visibility.get("cueWindowGateActive"),
            "win_sourceCellGateActive": win_visibility.get("sourceCellGateActive"),
            "win_materializedGateActive": win_visibility.get("materializedGateActive"),
        },
        "row_mode": {
            "psx": psx_record["rail_data_source"]["row_mode"],
            "win_rail": win_rail.get("rowMode"),
            "win_cursor": win_cursor.get("rowMode"),
            "rail_match": _compare_values(
                psx_record["rail_data_source"]["row_mode"],
                win_rail.get("rowMode"),
            ),
            "cursor_match": _compare_values(
                psx_record["rail_data_source"]["row_mode"],
                win_cursor.get("rowMode"),
            ),
        },
    }


def _timeline_anchor_psx_only(psx_record: dict[str, Any]) -> dict[str, Any]:
    overlay = (psx_record.get("rail_data_source") or {}).get("overlay") or {}
    resolved = overlay.get("resolved_event") or {}
    return {
        "psx_tick96": psx_record["timing"]["tick96"],
        "psx_bucket": psx_record["timing"]["bucket"],
        "psx_main_event_cursor": psx_record["timing"]["main_event_cursor"],
        "psx_main_event_count": psx_record["timing"]["main_event_count"],
        "psx_overlay_stream_id": psx_record["timing"]["overlay_stream_id"],
        "psx_overlay_event_frame": psx_record["timing"]["overlay_event_frame"],
        "psx_overlay_event_index": resolved.get("event_index"),
        "psx_overlay_event_addr": resolved.get("event_psx_addr"),
    }


def _dedupe_reasons(reasons: list[str]) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for reason in reasons:
        if reason in seen:
            continue
        seen.add(reason)
        out.append(reason)
    return out


def _normalize_main_event(event: dict[str, Any] | None) -> dict[str, Any] | None:
    if not event:
        return None
    return {
        "index": event.get("index"),
        "tick": event.get("tick"),
        "mask": event.get("mask"),
    }


def _focus_actor(actor: dict[str, Any]) -> dict[str, Any]:
    return {
        "visible": actor.get("visible"),
        "cursor": actor.get("cursor"),
        "enable": actor.get("enable"),
        "frame": actor.get("frame"),
        "x": actor.get("x"),
        "y": actor.get("y"),
    }


def _build_psx_focus_entry(
    sample_index: int,
    elapsed_ms: int,
    timeline_anchor: dict[str, Any],
    reasons: list[str],
    psx_record: dict[str, Any],
) -> dict[str, Any]:
    data_source = psx_record["rail_data_source"]
    rail_display = psx_record["rail_display"]
    overlay = data_source.get("overlay") or {}
    resolved = overlay.get("resolved_event") or {}
    current_event = _normalize_main_event(((psx_record.get("rail_payloads") or {}).get("current") or {}).get("timing"))
    next_event = _normalize_main_event(((psx_record.get("rail_payloads") or {}).get("next") or {}).get("timing"))
    rail_sfx = psx_record.get("rail_sfx") or {}
    voice_lane = rail_sfx.get("voice_lane") or {}
    late_branch = data_source.get("late_branch") or {}
    return {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_psx_focus_sample_v1",
        "sample_index": sample_index,
        "elapsed_ms": elapsed_ms,
        "timeline_anchor": timeline_anchor,
        "reasons": list(reasons),
        "timing": {
            "tick96": psx_record["timing"]["tick96"],
            "bucket": psx_record["timing"]["bucket"],
            "row_mode": data_source.get("row_mode"),
            "main_event_cursor": psx_record["timing"]["main_event_cursor"],
            "main_event_count": psx_record["timing"]["main_event_count"],
        },
        "display": {
            "rail_visible": rail_display.get("visible"),
            "display_mode": rail_display.get("display_mode"),
            "icon_codes": rail_display.get("icon_codes"),
            "teacher": _focus_actor(rail_display["teacher_portrait"]),
            "student": _focus_actor(rail_display["student_portrait"]),
        },
        "streams": {
            "icon_primary_stream18": data_source.get("icon_stream_primary_stream18"),
            "icon_alt_stream18": data_source.get("icon_stream_alt_stream18"),
            "body_primary_stream18": data_source.get("body_stream_primary_stream18"),
            "body_alt_stream18": data_source.get("body_stream_alt_stream18"),
        },
        "main_events": {
            "current": current_event,
            "next": next_event,
        },
        "overlay_event": {
            "stream_id": overlay.get("active_stream_id"),
            "event_index": resolved.get("event_index"),
            "frame": resolved.get("frame"),
            "event_psx_addr": resolved.get("event_psx_addr"),
        },
        "source_link": {
            "selected_stream": late_branch.get("selected_stream"),
            "selected_row": late_branch.get("selected_row"),
            "follow_up_phase_coarse": (rail_sfx.get("follow_up_phase_coarse")),
        },
        "rail_sfx": {
            "dispatch_available": rail_sfx.get("dispatch_available"),
            "trigger": rail_sfx.get("trigger"),
            "voice_lane": {
                "replace_restart_handle": voice_lane.get("replace_restart_handle"),
                "program": voice_lane.get("program"),
                "note": voice_lane.get("note"),
                "key": voice_lane.get("key"),
                "volume": voice_lane.get("volume"),
            },
        },
    }


def _build_psx_only_record(
    sample_index: int,
    elapsed_ms: int,
    psx_record: dict[str, Any],
) -> dict[str, Any]:
    reasons = _dedupe_reasons(list(psx_record.get("reasons") or []))
    if sample_index == 0 and "capture_start" not in reasons:
        reasons.insert(0, "capture_start")
    return {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_full_flow_capture_v2",
        "capture_mode": "psx-only",
        "sample_index": sample_index,
        "elapsed_ms": elapsed_ms,
        "capture_timing": {},
        "timeline_anchor": _timeline_anchor_psx_only(psx_record),
        "reasons": reasons,
        "psx": psx_record,
    }


def _cue_signature(cue: dict[str, Any] | None) -> tuple[Any, Any]:
    cue = cue or {}
    return (cue.get("available"), cue.get("symbolType"))


def _actor_signature(actor: dict[str, Any] | None) -> tuple[Any, ...]:
    actor = actor or {}
    return (
        actor.get("available"),
        actor.get("enable"),
        actor.get("portraitVisible"),
        actor.get("effectiveCursor"),
        actor.get("rollFrame"),
        actor.get("portraitCenterX"),
    )


def _append_change(reasons: list[str], prev_value: Any, curr_value: Any, tag: str) -> None:
    if prev_value != curr_value:
        reasons.append(tag)


def _build_dual_reasons(
    psx_record: dict[str, Any],
    win_timeline: dict[str, Any],
    comparison: dict[str, Any],
    prev_combined: dict[str, Any] | None,
) -> list[str]:
    reasons = _dedupe_reasons(list(psx_record.get("reasons") or []))
    if prev_combined is None:
        return reasons or ["capture_start"]

    prev_win = prev_combined["win_rail"]
    prev_comp = prev_combined["comparison"]
    _append_change(reasons, prev_win.get("parse_ok"), win_timeline.get("parse_ok"), "win_parse_state_change")
    _append_change(
        reasons,
        (prev_win.get("rail") or {}).get("valid"),
        (win_timeline.get("rail") or {}).get("valid"),
        "win_rail_valid_change",
    )
    _append_change(
        reasons,
        (prev_win.get("rail") or {}).get("rowMode"),
        (win_timeline.get("rail") or {}).get("rowMode"),
        "win_row_mode_change",
    )
    _append_change(
        reasons,
        tuple((prev_win.get("rail") or {}).get("bodyStreamA") or ()),
        tuple((win_timeline.get("rail") or {}).get("bodyStreamA") or ()),
        "win_body_stream_change",
    )
    _append_change(
        reasons,
        _cue_signature((prev_win.get("rail") or {}).get("currentCommittedCue")),
        _cue_signature((win_timeline.get("rail") or {}).get("currentCommittedCue")),
        "win_current_cue_change",
    )
    _append_change(
        reasons,
        (
            _cue_signature((prev_win.get("rail") or {}).get("nextLookaheadCue")),
            (prev_win.get("rail") or {}).get("nextLookaheadCueWindowActive"),
            (prev_win.get("rail") or {}).get("nextLookaheadAcceptedContributionEnabled"),
        ),
        (
            _cue_signature((win_timeline.get("rail") or {}).get("nextLookaheadCue")),
            (win_timeline.get("rail") or {}).get("nextLookaheadCueWindowActive"),
            (win_timeline.get("rail") or {}).get("nextLookaheadAcceptedContributionEnabled"),
        ),
        "win_next_cue_change",
    )
    _append_change(
        reasons,
        (
            _cue_signature((prev_win.get("rail") or {}).get("acceptedBoundaryCue")),
            (prev_win.get("rail") or {}).get("acceptedBoundarySourceCellGateActive"),
            (prev_win.get("rail") or {}).get("acceptedBoundaryMaterialized"),
            ((prev_win.get("rail") or {}).get("acceptedHandoffPhase") or {}).get("value"),
        ),
        (
            _cue_signature((win_timeline.get("rail") or {}).get("acceptedBoundaryCue")),
            (win_timeline.get("rail") or {}).get("acceptedBoundarySourceCellGateActive"),
            (win_timeline.get("rail") or {}).get("acceptedBoundaryMaterialized"),
            ((win_timeline.get("rail") or {}).get("acceptedHandoffPhase") or {}).get("value"),
        ),
        "win_accepted_cue_change",
    )
    _append_change(
        reasons,
        _actor_signature((prev_win.get("cursor") or {}).get("teacher")),
        _actor_signature((win_timeline.get("cursor") or {}).get("teacher")),
        "win_teacher_handoff_change",
    )
    _append_change(
        reasons,
        _actor_signature((prev_win.get("cursor") or {}).get("student")),
        _actor_signature((win_timeline.get("cursor") or {}).get("student")),
        "win_student_handoff_change",
    )
    _append_change(
        reasons,
        (
            (prev_win.get("displayGate") or {}).get("lessonSelector"),
            (prev_win.get("displayGate") or {}).get("lessonVisible"),
            (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("narrowVisibleCandidate")),
            (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("cueWindowGateActive")),
            (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("sourceCellGateActive")),
            (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("materializedGateActive")),
        ),
        (
            (win_timeline.get("displayGate") or {}).get("lessonSelector"),
            (win_timeline.get("displayGate") or {}).get("lessonVisible"),
            (((win_timeline.get("displayGate") or {}).get("railVisibility") or {}).get("narrowVisibleCandidate")),
            (((win_timeline.get("displayGate") or {}).get("railVisibility") or {}).get("cueWindowGateActive")),
            (((win_timeline.get("displayGate") or {}).get("railVisibility") or {}).get("sourceCellGateActive")),
            (((win_timeline.get("displayGate") or {}).get("railVisibility") or {}).get("materializedGateActive")),
        ),
        "win_display_gate_change",
    )
    _append_change(
        reasons,
        prev_comp["ahead_of_time_body"]["exact_match"],
        comparison["ahead_of_time_body"]["exact_match"],
        "body_alignment_change",
    )
    _append_change(
        reasons,
        (
            prev_comp["portrait_handoff"]["teacher"]["cursor_match"],
            prev_comp["portrait_handoff"]["teacher"]["visibility_match"],
            prev_comp["portrait_handoff"]["student"]["cursor_match"],
            prev_comp["portrait_handoff"]["student"]["visibility_match"],
        ),
        (
            comparison["portrait_handoff"]["teacher"]["cursor_match"],
            comparison["portrait_handoff"]["teacher"]["visibility_match"],
            comparison["portrait_handoff"]["student"]["cursor_match"],
            comparison["portrait_handoff"]["student"]["visibility_match"],
        ),
        "portrait_alignment_change",
    )
    return _dedupe_reasons(reasons)


def _build_combined_record(
    sample_index: int,
    elapsed_ms: int,
    psx_record: dict[str, Any],
    win_sample: dict[str, Any],
    prev_combined: dict[str, Any] | None,
) -> dict[str, Any]:
    win_timeline = win_sample["timeline"]
    comparison = build_dual_comparison(psx_record, win_timeline)
    reasons = _build_dual_reasons(psx_record, win_timeline, comparison, prev_combined)
    return {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_full_flow_capture_v2",
        "capture_mode": "dual",
        "sample_index": sample_index,
        "elapsed_ms": elapsed_ms,
        "capture_timing": {
            "win_rail_query_ms": win_sample["timing"]["rail_query_ms"],
            "win_status_query_ms": win_sample["timing"]["status_query_ms"],
        },
        "timeline_anchor": comparison["timeline_bridge"],
        "reasons": reasons,
        "psx": psx_record,
        "win_rail": win_timeline,
        "comparison": comparison,
    }


def _write_jsonl_line(path: Path, record: dict[str, Any]) -> None:
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(record, ensure_ascii=False) + "\n")


def _write_json(path: Path, payload: Any) -> None:
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")


def _unique_output_dir(base_dir: Path, label: str) -> Path:
    stamp = _dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    safe = "".join(ch if ch.isalnum() or ch in ("-", "_") else "_" for ch in label).strip("_") or "stage1_rail"
    out_dir = base_dir / f"{stamp}_{safe}"
    suffix = 1
    while out_dir.exists():
        out_dir = base_dir / f"{stamp}_{safe}_{suffix}"
        suffix += 1
    out_dir.mkdir(parents=True, exist_ok=False)
    return out_dir


def _summary_text(output_dir: Path, session_payload: dict[str, Any]) -> str:
    summary = session_payload["summary"]
    files = session_payload["files"]
    analysis_counts = session_payload.get("analysis_index", {}).get("counts", {})
    lines = [
        "Stage1 rail capture v2",
        f"capture_mode={session_payload['capture_mode']}",
        f"captured_at={session_payload['captured_at']}",
        f"label={session_payload['label']}",
        f"samples={summary['samples_captured']}",
        f"keyframes={summary['keyframes_emitted']}",
        f"psx_visible_samples={summary['psx_visible_samples']}",
        f"teacher_visible_samples={summary['teacher_visible_samples']}",
        f"student_visible_samples={summary['student_visible_samples']}",
        f"psx_sfx_events={summary['psx_sfx_events']}",
        f"psx_buckets_seen={summary['psx_buckets_seen']}",
        f"psx_row_modes_seen={summary['psx_row_modes_seen']}",
        f"psx_overlay_streams_seen={summary['psx_overlay_streams_seen']}",
        f"psx_resolved_overlay_samples={summary['psx_resolved_overlay_samples']}",
        f"icon_stream_change_samples={analysis_counts.get('icon_stream_change_samples', 0)}",
        f"body_stream_change_samples={analysis_counts.get('body_stream_change_samples', 0)}",
        f"main_event_current_change_samples={analysis_counts.get('main_event_current_change_samples', 0)}",
        f"main_event_next_change_samples={analysis_counts.get('main_event_next_change_samples', 0)}",
    ]
    if session_payload["capture_mode"] != "psx-only":
        lines.extend(
            [
                f"win_runtime_available_samples={summary['win_runtime_available_samples']}",
                f"win_rail_valid_samples={summary['win_rail_valid_samples']}",
                f"win_lesson_visible_samples={summary['win_lesson_visible_samples']}",
                f"body_stream_match_samples={summary['body_stream_match_samples']}/{summary['body_stream_compare_samples']}",
                f"teacher_cursor_match_samples={summary['teacher_cursor_match_samples']}/{summary['teacher_cursor_compare_samples']}",
                f"student_cursor_match_samples={summary['student_cursor_match_samples']}/{summary['student_cursor_compare_samples']}",
                f"win_row_modes_seen={summary['win_row_modes_seen']}",
                f"win_lesson_selectors_seen={summary['win_lesson_selectors_seen']}",
                f"accepted_handoff_phases_seen={summary['accepted_handoff_phases_seen']}",
            ]
        )
    else:
        lines.append("win_runtime=disabled")
    lines.extend(
        [
            f"timeline={files['timeline']}",
            f"focus_timeline={files['focus_timeline']}",
            f"sfx_timeline={files['sfx_timeline']}",
            f"keyframes={files['keyframes']}",
            f"analysis_index={files['analysis_index']}",
            f"session={files['session']}",
            f"artifact_dir={output_dir}",
        ]
    )
    return "\n".join(lines) + "\n"


def _scalar_counter_rows(counter: Counter[Any]) -> list[dict[str, Any]]:
    def _sort_key(item: tuple[Any, int]) -> tuple[int, Any]:
        value = item[0]
        if value is None:
            return (1, "")
        if isinstance(value, (int, float)):
            return (0, value)
        return (0, str(value))

    return [
        {"value": value, "count": count}
        for value, count in sorted(counter.items(), key=_sort_key)
    ]


def _event_counter_rows(counter: Counter[tuple[Any, Any, Any]]) -> list[dict[str, Any]]:
    def _sort_key(item: tuple[tuple[Any, Any, Any], int]) -> tuple[int, Any, Any, Any]:
        index, tick, mask = item[0]
        return (1 if index is None else 0, index if index is not None else -1, tick, mask)

    return [
        {
            "index": index,
            "tick": tick,
            "mask": mask,
            "count": count,
        }
        for (index, tick, mask), count in sorted(counter.items(), key=_sort_key)
    ]


def _voice_counter_rows(counter: Counter[tuple[int, int, int, int, int]]) -> list[dict[str, Any]]:
    return [
        {
            "replace_restart_handle": signature[0],
            "program": signature[1],
            "note": signature[2],
            "key": signature[3],
            "volume": signature[4],
            "count": count,
        }
        for signature, count in sorted(counter.items(), key=lambda item: (-item[1], item[0]))
    ]


def _self_test() -> int:
    raw = (
        "rail.valid=1 rail.queryFrame=456 rail.rowMode=1\n"
        "rail.bodyStreamA[18]=[-1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "currentCommittedCue.available=1 currentCommittedCue.symbolType=3\n"
        "nextLookaheadCue.available=1 nextLookaheadCue.symbolType=2 "
        "rail.nextLookaheadCueWindowActive=1 "
        "rail.nextLookaheadAcceptedContributionEnabled=0\n"
        "acceptedBoundaryCue.available=1 acceptedBoundaryCue.symbolType=4 "
        "rail.acceptedBoundarySourceCellGateActive=1 "
        "rail.acceptedBoundaryMaterialized=0 rail.acceptedHandoffPhase=SourceCellGate(2)\n"
        "cursor.valid=1 cursor.queryFrame=456 cursor.rowMode=1\n"
        "teacher.available=1 teacher.enable=1 teacher.portraitVisible=1 "
        "teacher.effectiveCursor=4 teacher.rollFrame=2 "
        "teacher.portraitCenterXValid=1 teacher.portraitCenterX=86\n"
        "student.available=1 student.enable=1 student.portraitVisible=1 "
        "student.effectiveCursor=7 student.rollFrame=3 "
        "student.portraitCenterXValid=1 student.portraitCenterX=131\n"
        "displayGate.valid=1 displayGate.queryFrame=456 displayGate.lessonSelector=2 "
        "displayGate.lessonVisible=1 "
        "displayGate.railVisibility.narrowVisibleCandidate=1 "
        "displayGate.railVisibility.cueWindowGateActive=1 "
        "displayGate.railVisibility.sourceCellGateActive=0 "
        "displayGate.railVisibility.materializedGateActive=0"
    )
    parsed = parse_stage1ovl_rail(raw)
    assert parsed["parse_ok"] is True
    assert parsed["rail"]["queryFrame"] == 456
    assert parsed["rail"]["bodyStreamA"][1] == 2
    assert parsed["rail"]["acceptedHandoffPhase"]["name"] == "SourceCellGate"

    portrait_anim = {"teacher": {}, "student": {}}
    base = {
        "ctx": {"tick96": 120, "current_bucket": 10},
        "overlay": {
            "main_event_cursor": 3,
            "main_event_count": 5,
            "active_stream_id": 1,
            "resolved_event": {"frame": 84},
        },
        "late_branch": {
            "selected_stream": 3,
            "selected_row": 8,
            "selected_row_handles": [1, 2, 0, 0, 0],
            "selected_row_names": ["A", "B", None, None, None],
            "selected_row_ptrs": [0x80100000, 0x80100020, 0, 0, 0],
            "consumer_row_handles": [1, 2, 0, 0, 0],
            "consumer_row_ptrs": [0x80100000, 0x80100020, 0, 0, 0],
            "follow_up_phase_coarse": 1,
        },
        "accepted_formula": {
            "accepted_mask_accum": 0xE0,
            "accepted_contribution_count": 3,
            "recorded_split_count": 1,
            "penalty_split_count": 2,
            "score_accumulator": 0,
            "good_to_cool_gate_enabled": True,
        },
        "voice_lane": {
            "replace_restart_handle": 7,
            "program": 2,
            "note": 60,
            "key": 84,
            "volume": 100,
        },
    }
    raw_psx = {
        "row_mode": 1,
        "teacher_cursor": 4,
        "teacher_enable": 1,
        "student_cursor": 7,
        "student_enable": 1,
        "stream94": {"stream18": [1, 2, 3, 4] + [0] * 14},
        "stream98": None,
        "streamA4": {"stream18": [-1, 2, 3, 4] + [0] * 14},
        "streamA8": None,
    }
    psx_record = _build_record(
        sample_index=0,
        elapsed_ms=0,
        base_snapshot=base,
        raw=raw_psx,
        main_event_current={"index": 3, "tick": 96, "mask": 0x40},
        main_event_next={"index": 4, "tick": 108, "mask": 0x20},
        portrait_anim=portrait_anim,
        prev_record=None,
    )
    win_timeline = _timeline_win_entry(parsed, {"stageFrame": 222})
    comparison = build_dual_comparison(psx_record, win_timeline)
    assert comparison["ahead_of_time_body"]["exact_match"] is True
    assert comparison["portrait_handoff"]["teacher"]["cursor_match"] is True
    psx_only_record = _build_psx_only_record(0, 0, psx_record)
    assert psx_only_record["capture_mode"] == "psx-only"
    assert psx_only_record["timeline_anchor"]["psx_main_event_cursor"] == 3
    focus_entry = _build_psx_focus_entry(0, 0, psx_only_record["timeline_anchor"], psx_only_record["reasons"], psx_record)
    assert focus_entry["timing"]["row_mode"] == 1
    assert focus_entry["main_events"]["current"]["index"] == 3
    assert focus_entry["display"]["teacher"]["x"] == 76
    quiet_record = dict(psx_record)
    quiet_record["reasons"] = []
    assert _build_psx_only_record(1, 33, quiet_record)["reasons"] == []
    print("[SELFTEST] ok")
    return 0


def capture(args: argparse.Namespace) -> int:
    overlay = dual.load_overlay_model(args.comod, args.compo)
    output_dir = _unique_output_dir(args.output_dir, args.label)
    timeline_path = output_dir / "timeline_v2.jsonl"
    focus_timeline_path = output_dir / "psx_focus_timeline.jsonl"
    sfx_timeline_path = output_dir / "psx_sfx_timeline.jsonl"
    keyframes_path = output_dir / "keyframes.json"
    analysis_index_path = output_dir / "analysis_index.json"
    session_path = output_dir / "session.json"
    summary_path = output_dir / "summary.txt"

    reader = dual.GdbRunningReader(args.host, args.port)
    debug: DebugClient | None = None
    if not args.psx_only:
        debug = DebugClient(host=args.debug_host, port=args.debug_port, timeout=args.debug_timeout_sec)
    portrait_anim = {"teacher": {}, "student": {}}
    start_time = time.monotonic()
    last_wait_log_ms = -5000
    sample_index = 0
    started = False
    prev_psx_record: dict[str, Any] | None = None
    prev_combined: dict[str, Any] | None = None
    keyframes: list[dict[str, Any]] = []
    summary = {
        "samples_captured": 0,
        "psx_visible_samples": 0,
        "teacher_visible_samples": 0,
        "student_visible_samples": 0,
        "psx_sfx_events": 0,
        "win_runtime_available_samples": 0,
        "win_rail_valid_samples": 0,
        "win_lesson_visible_samples": 0,
        "win_parse_error_samples": 0,
        "body_stream_compare_samples": 0,
        "body_stream_match_samples": 0,
        "teacher_cursor_compare_samples": 0,
        "teacher_cursor_match_samples": 0,
        "student_cursor_compare_samples": 0,
        "student_cursor_match_samples": 0,
        "psx_display_modes_seen": set(),
        "psx_buckets_seen": set(),
        "psx_row_modes_seen": set(),
        "psx_overlay_streams_seen": set(),
        "psx_resolved_overlay_samples": 0,
        "win_row_modes_seen": set(),
        "win_lesson_selectors_seen": set(),
        "accepted_handoff_phases_seen": set(),
        "keyframes_emitted": 0,
    }
    reason_counts: Counter[str] = Counter()
    reason_markers: dict[str, list[dict[str, Any]]] = {}
    display_mode_counts: Counter[Any] = Counter()
    bucket_counts: Counter[Any] = Counter()
    row_mode_counts: Counter[Any] = Counter()
    teacher_cursor_counts: Counter[Any] = Counter()
    student_cursor_counts: Counter[Any] = Counter()
    current_main_event_counts: Counter[tuple[Any, Any, Any]] = Counter()
    next_main_event_counts: Counter[tuple[Any, Any, Any]] = Counter()
    sfx_signature_counts: Counter[tuple[int, int, int, int, int]] = Counter()

    try:
        while not STOP_REQUESTED:
            loop_begin = time.monotonic()
            base_snapshot = dual.read_snapshot(reader, overlay)
            elapsed_ms = int((loop_begin - start_time) * 1000.0)
            mainloop_active = base_snapshot["ctx"]["mainloop_active"] == 1

            if not started:
                if args.no_wait or mainloop_active:
                    started = True
                else:
                    if elapsed_ms - last_wait_log_ms >= 5000:
                        last_wait_log_ms = elapsed_ms
                        print(
                            f"[WAIT] ms={elapsed_ms} tick96={base_snapshot['ctx']['tick96']} "
                            f"scene_ptr=0x{base_snapshot['scene_ptr']:08X}"
                        )
                    time.sleep(args.sample_ms / 1000.0)
                    continue

            if started and args.stop_on_gameplay_end and not mainloop_active and summary["samples_captured"] > 0:
                print(f"[INFO] gameplay gate dropped at {elapsed_ms}ms; stopping")
                break

            raw = _read_stage1_rail_raw(reader)
            event_cursor = int(base_snapshot["overlay"]["main_event_cursor"])
            main_event_current = _read_main_event(reader, event_cursor if event_cursor >= 0 else -1)
            main_event_next = _read_main_event(reader, event_cursor + 1)
            psx_record = _build_record(
                sample_index=sample_index,
                elapsed_ms=elapsed_ms,
                base_snapshot=base_snapshot,
                raw=raw,
                main_event_current=main_event_current,
                main_event_next=main_event_next,
                portrait_anim=portrait_anim,
                prev_record=prev_psx_record,
            )
            if args.psx_only:
                combined = _build_psx_only_record(
                    sample_index=sample_index,
                    elapsed_ms=elapsed_ms,
                    psx_record=psx_record,
                )
            else:
                assert debug is not None
                win_sample = query_win_stage1_rail(debug)
                combined = _build_combined_record(
                    sample_index=sample_index,
                    elapsed_ms=elapsed_ms,
                    psx_record=psx_record,
                    win_sample=win_sample,
                    prev_combined=prev_combined,
                )
            combined["capture_timing"]["sample_duration_ms"] = int((time.monotonic() - loop_begin) * 1000.0)

            _write_jsonl_line(timeline_path, combined)
            focus_entry = _build_psx_focus_entry(
                sample_index=sample_index,
                elapsed_ms=elapsed_ms,
                timeline_anchor=combined["timeline_anchor"],
                reasons=combined["reasons"],
                psx_record=psx_record,
            )
            _write_jsonl_line(focus_timeline_path, focus_entry)
            if "rail_sfx_change" in combined["reasons"] and psx_record["rail_sfx"]["dispatch_available"]:
                _write_jsonl_line(
                    sfx_timeline_path,
                    {
                        "api": "openai-responses",
                        "sample_index": sample_index,
                        "elapsed_ms": elapsed_ms,
                        "timeline_anchor": combined["timeline_anchor"],
                        "psx_rail_sfx": psx_record["rail_sfx"],
                    },
                )
                summary["psx_sfx_events"] += 1

            if combined["reasons"]:
                keyframes.append(
                    {
                        "sample_index": sample_index,
                        "elapsed_ms": elapsed_ms,
                        "timeline_anchor": combined["timeline_anchor"],
                        "reasons": combined["reasons"],
                        "focus": focus_entry,
                    }
                )
                marker = {
                    "sample_index": sample_index,
                    "elapsed_ms": elapsed_ms,
                    "tick96": focus_entry["timing"]["tick96"],
                    "bucket": focus_entry["timing"]["bucket"],
                    "row_mode": focus_entry["timing"]["row_mode"],
                }
                for reason in combined["reasons"]:
                    reason_counts[reason] += 1
                    reason_markers.setdefault(reason, []).append(dict(marker))

            summary["samples_captured"] += 1
            summary["psx_visible_samples"] += int(psx_record["rail_display"]["visible"])
            summary["teacher_visible_samples"] += int(psx_record["rail_display"]["teacher_portrait"]["visible"])
            summary["student_visible_samples"] += int(psx_record["rail_display"]["student_portrait"]["visible"])
            summary["psx_display_modes_seen"].add(psx_record["rail_display"]["display_mode"])
            if psx_record["timing"]["bucket"] is not None:
                summary["psx_buckets_seen"].add(psx_record["timing"]["bucket"])
            if psx_record["rail_data_source"]["row_mode"] is not None:
                summary["psx_row_modes_seen"].add(psx_record["rail_data_source"]["row_mode"])
            if psx_record["timing"]["overlay_stream_id"] is not None:
                summary["psx_overlay_streams_seen"].add(psx_record["timing"]["overlay_stream_id"])
            if ((psx_record["rail_data_source"].get("overlay") or {}).get("resolved_event")) is not None:
                summary["psx_resolved_overlay_samples"] += 1
            display_mode_counts[psx_record["rail_display"]["display_mode"]] += 1
            bucket_counts[psx_record["timing"]["bucket"]] += 1
            row_mode_counts[psx_record["rail_data_source"]["row_mode"]] += 1
            teacher_cursor_counts[psx_record["rail_display"]["teacher_portrait"]["cursor"]] += 1
            student_cursor_counts[psx_record["rail_display"]["student_portrait"]["cursor"]] += 1
            current_main_event_counts[_main_event_signature(focus_entry["main_events"]["current"])] += 1
            next_main_event_counts[_main_event_signature(focus_entry["main_events"]["next"])] += 1
            if psx_record["rail_sfx"]["dispatch_available"]:
                sfx_signature_counts[_voice_signature(psx_record["rail_sfx"]["voice_lane"])] += 1

            if not args.psx_only:
                win_timeline = combined["win_rail"]
                if win_timeline["runtime_available"]:
                    summary["win_runtime_available_samples"] += 1
                if not win_timeline["parse_ok"]:
                    summary["win_parse_error_samples"] += 1
                if (win_timeline.get("rail") or {}).get("valid"):
                    summary["win_rail_valid_samples"] += 1
                if (win_timeline.get("displayGate") or {}).get("lessonVisible"):
                    summary["win_lesson_visible_samples"] += 1
                win_row_mode = (win_timeline.get("rail") or {}).get("rowMode")
                if win_row_mode is not None:
                    summary["win_row_modes_seen"].add(win_row_mode)
                lesson_selector = (win_timeline.get("displayGate") or {}).get("lessonSelector")
                if lesson_selector is not None:
                    summary["win_lesson_selectors_seen"].add(lesson_selector)
                handoff_name = ((win_timeline.get("rail") or {}).get("acceptedHandoffPhase") or {}).get("name")
                if handoff_name:
                    summary["accepted_handoff_phases_seen"].add(handoff_name)

                comparison = combined["comparison"]
                body_match = comparison["ahead_of_time_body"]["exact_match"]
                if body_match is not None:
                    summary["body_stream_compare_samples"] += 1
                    summary["body_stream_match_samples"] += int(bool(body_match))
                teacher_match = comparison["portrait_handoff"]["teacher"]["cursor_match"]
                if teacher_match is not None:
                    summary["teacher_cursor_compare_samples"] += 1
                    summary["teacher_cursor_match_samples"] += int(bool(teacher_match))
                student_match = comparison["portrait_handoff"]["student"]["cursor_match"]
                if student_match is not None:
                    summary["student_cursor_compare_samples"] += 1
                    summary["student_cursor_match_samples"] += int(bool(student_match))

            prev_psx_record = psx_record
            prev_combined = combined
            sample_index += 1

            if args.seconds > 0 and elapsed_ms >= args.seconds * 1000:
                print(f"[INFO] reached --seconds={args.seconds}")
                break

            sleep_sec = max(0.0, args.sample_ms / 1000.0 - (time.monotonic() - loop_begin))
            if sleep_sec > 0:
                time.sleep(sleep_sec)
    finally:
        reader.close()

    for key in (
        "psx_display_modes_seen",
        "psx_buckets_seen",
        "psx_row_modes_seen",
        "psx_overlay_streams_seen",
        "win_row_modes_seen",
        "win_lesson_selectors_seen",
        "accepted_handoff_phases_seen",
    ):
        summary[key] = sorted(summary[key])
    summary["keyframes_emitted"] = len(keyframes)

    capture_mode = "psx-only" if args.psx_only else "dual"
    prerequisites = [
        "DuckStation GDB server reachable on the configured host/port",
        "Stage1 session running or ready for manual play",
    ]
    if not args.psx_only:
        prerequisites.insert(1, "Win debug server reachable on the configured host/port")

    analysis_index = {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_full_flow_capture_v2",
        "capture_mode": capture_mode,
        "counts": {
            "samples_captured": summary["samples_captured"],
            "keyframes_emitted": summary["keyframes_emitted"],
            "psx_visible_samples": summary["psx_visible_samples"],
            "teacher_visible_samples": summary["teacher_visible_samples"],
            "student_visible_samples": summary["student_visible_samples"],
            "psx_sfx_events": summary["psx_sfx_events"],
            "icon_stream_change_samples": reason_counts.get("icon_stream_change", 0),
            "body_stream_change_samples": reason_counts.get("body_stream_change", 0),
            "body_stream_alt_change_samples": reason_counts.get("body_stream_alt_change", 0),
            "row_mode_change_samples": reason_counts.get("row_mode_change", 0),
            "teacher_cursor_change_samples": reason_counts.get("teacher_cursor_change", 0),
            "student_cursor_change_samples": reason_counts.get("student_cursor_change", 0),
            "visibility_change_samples": reason_counts.get("visibility_change", 0),
            "main_event_current_change_samples": reason_counts.get("main_event_current_change", 0),
            "main_event_next_change_samples": reason_counts.get("main_event_next_change", 0),
            "rail_sfx_change_samples": reason_counts.get("rail_sfx_change", 0),
        },
        "histograms": {
            "display_mode": _scalar_counter_rows(display_mode_counts),
            "bucket": _scalar_counter_rows(bucket_counts),
            "row_mode": _scalar_counter_rows(row_mode_counts),
            "teacher_cursor": _scalar_counter_rows(teacher_cursor_counts),
            "student_cursor": _scalar_counter_rows(student_cursor_counts),
        },
        "main_events": {
            "current": _event_counter_rows(current_main_event_counts),
            "next": _event_counter_rows(next_main_event_counts),
        },
        "sfx_signatures": _voice_counter_rows(sfx_signature_counts),
        "reason_index": {
            reason: {
                "count": reason_counts[reason],
                "markers": reason_markers.get(reason, []),
            }
            for reason in sorted(reason_counts)
        },
    }

    session_payload = {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_full_flow_capture_v2",
        "capture_mode": capture_mode,
        "captured_at": _dt.datetime.now().isoformat(),
        "label": args.label,
        "gdb": {"host": args.host, "port": args.port},
        "debug_server": (
            {"enabled": False, "host": None, "port": None}
            if args.psx_only
            else {"enabled": True, "host": args.debug_host, "port": args.debug_port}
        ),
        "sample_ms": args.sample_ms,
        "seconds": args.seconds,
        "wait_for_gameplay_gate": not args.no_wait,
        "stop_on_gameplay_end": args.stop_on_gameplay_end,
        "files": {
            "timeline": str(timeline_path),
            "focus_timeline": str(focus_timeline_path),
            "sfx_timeline": str(sfx_timeline_path),
            "keyframes": str(keyframes_path),
            "analysis_index": str(analysis_index_path),
            "session": str(session_path),
            "summary": str(summary_path),
        },
        "summary": summary,
        "analysis_index": analysis_index,
        "prerequisites": prerequisites,
    }
    _write_json(keyframes_path, keyframes)
    _write_json(analysis_index_path, analysis_index)
    _write_json(session_path, session_payload)
    summary_path.write_text(_summary_text(output_dir, session_payload), encoding="utf-8")
    print(f"[OK] wrote {output_dir}")
    return 0


def build_argparser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Stage1 rail full-flow capture v2 (PSX-only by default)")
    parser.add_argument("--host", default=dual.GDB_HOST)
    parser.add_argument("--port", type=int, default=dual.GDB_PORT)
    parser.add_argument("--debug-host", default=DEBUG_HOST)
    parser.add_argument("--debug-port", type=int, default=DEBUG_PORT)
    parser.add_argument("--debug-timeout-sec", type=float, default=2.0)
    mode_group = parser.add_mutually_exclusive_group()
    mode_group.add_argument(
        "--psx-only",
        dest="psx_only",
        action="store_true",
        help="Capture DuckStation/PSX rail state only and skip all Win DebugServer queries (default)",
    )
    mode_group.add_argument(
        "--with-win",
        dest="psx_only",
        action="store_false",
        help="Also query the Win DebugServer and emit dual comparison artifacts",
    )
    parser.add_argument("--sample-ms", type=int, default=33)
    parser.add_argument("--seconds", type=int, default=0, help="0 means until gameplay exits or Ctrl+C")
    parser.add_argument("--label", default="manual_stage1_rail_full_psx_only")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--comod", type=Path, default=dual.DEFAULT_COMOD)
    parser.add_argument("--compo", type=Path, default=dual.DEFAULT_COMPO)
    parser.add_argument("--no-wait", action="store_true", help="Start immediately instead of waiting for gameplay gate")
    parser.add_argument("--no-stop-on-gameplay-end", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    parser.set_defaults(psx_only=True)
    return parser


def main() -> int:
    parser = build_argparser()
    args = parser.parse_args()
    if args.self_test:
        return _self_test()
    args.stop_on_gameplay_end = not args.no_stop_on_gameplay_end
    args.output_dir.mkdir(parents=True, exist_ok=True)
    return capture(args)


if __name__ == "__main__":
    raise SystemExit(main())
