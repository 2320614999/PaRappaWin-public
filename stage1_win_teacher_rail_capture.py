#!/usr/bin/env python3
"""Win-only high-frequency Stage1 teacher rail capture."""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import re
import signal
import time
from pathlib import Path
from typing import Any

from test_dispatcher import DispatcherTester


REPO_ROOT = Path(__file__).resolve().parent
DEFAULT_OUTPUT_DIR = REPO_ROOT / "artifacts" / "stage1_win_teacher_rail_capture"
CAPTURE_KIND = "stage1_win_teacher_rail_capture"

STOP_REQUESTED = False


def _signal_handler(signum: int, frame: Any) -> None:
    del signum, frame
    global STOP_REQUESTED
    STOP_REQUESTED = True


signal.signal(signal.SIGINT, _signal_handler)
signal.signal(signal.SIGTERM, _signal_handler)


def _write_json(path: Path, payload: Any) -> None:
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")


def _write_jsonl_line(path: Path, payload: Any) -> None:
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(payload, ensure_ascii=False) + "\n")


def _unique_output_dir(base_dir: Path, label: str) -> Path:
    stamp = _dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    safe = "".join(ch if ch.isalnum() or ch in ("-", "_") else "_" for ch in label).strip("_")
    if not safe:
        safe = "stage1_teacher_rail"
    out_dir = base_dir / f"{stamp}_{safe}"
    suffix = 1
    while out_dir.exists():
        out_dir = base_dir / f"{stamp}_{safe}_{suffix}"
        suffix += 1
    out_dir.mkdir(parents=True, exist_ok=False)
    return out_dir


def _search_int(src: str, pattern: str) -> int | None:
    match = re.search(pattern, src)
    if not match:
        return None
    return int(match.group(1))


def _search_hex(src: str, pattern: str) -> int | None:
    match = re.search(pattern, src)
    if not match:
        return None
    return int(match.group(1), 16)


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
        if token:
            out.append(int(token))
    return out


def _int_to_bool(value: int | None) -> bool | None:
    if value is None:
        return None
    return bool(value)


def _parse_named_enum(src: str, name: str) -> dict[str, Any]:
    match = re.search(rf"{re.escape(name)}=([A-Za-z_-]+)\((-?\d+)\)", src)
    return {
        "name": match.group(1) if match else None,
        "value": int(match.group(2)) if match else None,
    }


def _parse_identity_stream(src: str, name: str) -> list[int] | None:
    return _search_int_list(src, rf"{re.escape(name)}=\[(.*?)\]")


def _parse_cue(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "symbolType": _search_int(src, rf"{re.escape(name)}\.symbolType=(-?\d+)"),
    }


def _parse_descriptor(src: str, name: str) -> dict[str, Any]:
    return {
        "rowIndex": _search_int(src, rf"{re.escape(name)}\.rowIndex=(-?\d+)"),
        "selectorByte0": _search_int(src, rf"{re.escape(name)}\.selectorByte0=(-?\d+)"),
        "selectorByte1": _search_int(src, rf"{re.escape(name)}\.selectorByte1=(-?\d+)"),
        "symbolType": _search_int(src, rf"{re.escape(name)}\.symbolType=(-?\d+)"),
        "timingTemplateStateKnown": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.timingTemplateStateKnown=(-?\d+)")
        ),
        "timingTemplateState": _search_int(
            src, rf"{re.escape(name)}\.timingTemplateState=(-?\d+)"
        ),
        "sourceCellHeaderPresent": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.sourceCellHeaderPresent=(-?\d+)")
        ),
        "sourceCellHeaderCount": _search_int(
            src, rf"{re.escape(name)}\.sourceCellHeaderCount=(-?\d+)"
        ),
        "sourceCellHeaderCursor": _search_int(
            src, rf"{re.escape(name)}\.sourceCellHeaderCursor=(-?\d+)"
        ),
        "sourceCellPresent": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.sourceCellPresent=(-?\d+)")
        ),
    }


def _parse_actor(src: str, name: str) -> dict[str, Any]:
    current_enable = _search_int(src, rf"{re.escape(name)}\.currentEnable=(-?\d+)")
    if current_enable is None:
        current_enable = _search_int(src, rf"{re.escape(name)}\.enable=(-?\d+)")
    current_cursor = _search_int(src, rf"{re.escape(name)}\.currentCursor=(-?\d+)")
    if current_cursor is None:
        current_cursor = _search_int(src, rf"{re.escape(name)}\.cursor=(-?\d+)")
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "seeded": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.seeded=(-?\d+)")),
        "recordIndex": _search_int(src, rf"{re.escape(name)}\.recordIndex=(-?\d+)"),
        "secondaryRecordIndex": _search_int(src, rf"{re.escape(name)}\.secondaryRecordIndex=(-?\d+)"),
        "seedEnable": _search_int(src, rf"{re.escape(name)}\.seedEnable=(-?\d+)"),
        "startDueFrame": _search_int(src, rf"{re.escape(name)}\.startDueFrame=(-?\d+)"),
        "seedCursor": _search_int(src, rf"{re.escape(name)}\.seedCursor=(-?\d+)"),
        "currentCursor": current_cursor,
        "currentEnable": current_enable,
        "portraitVisible": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.portraitVisible=(-?\d+)")),
        "effectiveCursor": _search_int(src, rf"{re.escape(name)}\.effectiveCursor=(-?\d+)"),
        "rollFrame": _search_int(src, rf"{re.escape(name)}\.rollFrame=(-?\d+)"),
        "portraitCenterXValid": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.portraitCenterXValid=(-?\d+)")
        ),
        "portraitCenterX": _search_float(src, rf"{re.escape(name)}\.portraitCenterX=(-?\d+(?:\.\d+)?)"),
        "primaryStream18": _parse_identity_stream(src, f"{name}.primaryStream18"),
        "secondaryStream18": _parse_identity_stream(src, f"{name}.secondaryStream18"),
    }


def _parse_family_candidate(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "recordIndex": _search_int(src, rf"{re.escape(name)}\.recordIndex=(-?\d+)"),
        "secondaryRecordIndex": _search_int(src, rf"{re.escape(name)}\.secondaryRecordIndex=(-?\d+)"),
        "eventIndex": _search_int(src, rf"{re.escape(name)}\.eventIndex=(-?\d+)"),
        "familyStartIndex": _search_int(src, rf"{re.escape(name)}\.familyStartIndex=(-?\d+)"),
        "dueFrame": _search_int(src, rf"{re.escape(name)}\.dueFrame=(-?\d+)"),
        "familyStartDueFrame": _search_int(src, rf"{re.escape(name)}\.familyStartDueFrame=(-?\d+)"),
        "rowMode": _search_int(src, rf"{re.escape(name)}\.rowMode=(-?\d+)"),
        "streamA": _parse_identity_stream(src, f"{name}.streamA[18]"),
        "streamB": _parse_identity_stream(src, f"{name}.streamB[18]"),
    }


def _parse_preview_shadow(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "sourceKind": _parse_named_enum(src, f"{name}.sourceKind"),
        "sourceEventPsxAddr": _search_hex(src, rf"{re.escape(name)}\.sourceEventPsxAddr=0x([0-9A-Fa-f]+)"),
        "familyStartEventPsxAddr": _search_hex(
            src,
            rf"{re.escape(name)}\.familyStartEventPsxAddr=0x([0-9A-Fa-f]+)",
        ),
        "sourceEventIndex": _search_int(src, rf"{re.escape(name)}\.sourceEventIndex=(-?\d+)"),
        "familyStartIndex": _search_int(src, rf"{re.escape(name)}\.familyStartIndex=(-?\d+)"),
        "sourceDueFrame": _search_int(src, rf"{re.escape(name)}\.sourceDueFrame=(-?\d+)"),
        "startDueFrame": _search_int(src, rf"{re.escape(name)}\.startDueFrame=(-?\d+)"),
        "recordIndex": _search_int(src, rf"{re.escape(name)}\.recordIndex=(-?\d+)"),
        "secondaryRecordIndex": _search_int(src, rf"{re.escape(name)}\.secondaryRecordIndex=(-?\d+)"),
        "rowMode": _search_int(src, rf"{re.escape(name)}\.rowMode=(-?\d+)"),
        "seedCursor": _search_int(src, rf"{re.escape(name)}\.seedCursor=(-?\d+)"),
        "seedEnable": _search_int(src, rf"{re.escape(name)}\.seedEnable=(-?\d+)"),
        "firstVisibleCursor": _search_int(src, rf"{re.escape(name)}\.firstVisibleCursor=(-?\d+)"),
        "currentCursor": _search_int(src, rf"{re.escape(name)}\.currentCursor=(-?\d+)"),
        "currentEnable": _search_int(src, rf"{re.escape(name)}\.currentEnable=(-?\d+)"),
        "portraitVisible": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.portraitVisible=(-?\d+)")),
        "effectiveCursor": _search_int(src, rf"{re.escape(name)}\.effectiveCursor=(-?\d+)"),
        "rollFrame": _search_int(src, rf"{re.escape(name)}\.rollFrame=(-?\d+)"),
        "streamA": _parse_identity_stream(src, f"{name}.streamA[18]"),
        "streamB": _parse_identity_stream(src, f"{name}.streamB[18]"),
    }


def _parse_local_teacher_preview_probe(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "resolverImplKind": _search_int(src, rf"{re.escape(name)}\.resolverImplKind=(-?\d+)"),
        "blankWindowAccepted": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.blankWindowAccepted=(-?\d+)")
        ),
        "nextLookaheadHookFuturePromotable": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.nextLookaheadHookFuturePromotable=(-?\d+)")
        ),
        "visibleAnchorLatched": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.visibleAnchorLatched=(-?\d+)")
        ),
        "usedVisibleAnchor": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.usedVisibleAnchor=(-?\d+)")
        ),
        "preservedVisibleAnchor": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.preservedVisibleAnchor=(-?\d+)")
        ),
        "teacherPreviewCadenceGateArmed": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.teacherPreviewCadenceGateArmed=(-?\d+)")
        ),
        "teacherPreviewCadenceGateStartFrame": _search_int(
            src, rf"{re.escape(name)}\.teacherPreviewCadenceGateStartFrame=(-?\d+)"
        ),
        "teacherPreviewCadenceGatePhaseKey": _search_hex(
            src,
            rf"{re.escape(name)}\.teacherPreviewCadenceGatePhaseKey=0x([0-9A-Fa-f]+)",
        ),
        "sourceEventPsxAddr": _search_hex(
            src, rf"{re.escape(name)}\.sourceEventPsxAddr=0x([0-9A-Fa-f]+)"
        ),
        "sourceDueFrame": _search_int(src, rf"{re.escape(name)}\.sourceDueFrame=(-?\d+)"),
        "rowIndex": _search_int(src, rf"{re.escape(name)}\.rowIndex=(-?\d+)"),
        "selectorAvailable": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.selectorAvailable=(-?\d+)")
        ),
        "selectorByte0": _search_int(src, rf"{re.escape(name)}\.selectorByte0=(-?\d+)"),
        "selectorByte1": _search_int(src, rf"{re.escape(name)}\.selectorByte1=(-?\d+)"),
        "classToken": _search_int(src, rf"{re.escape(name)}\.classToken=(-?\d+)"),
        "sourceCellId": _search_hex(src, rf"{re.escape(name)}\.sourceCellId=0x([0-9A-Fa-f]+)"),
        "sourceCellHeaderPresent": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.sourceCellHeaderPresent=(-?\d+)")
        ),
        "sourceCellHeaderCount": _search_int(
            src, rf"{re.escape(name)}\.sourceCellHeaderCount=(-?\d+)"
        ),
        "sourceCellHeaderCursor": _search_int(
            src, rf"{re.escape(name)}\.sourceCellHeaderCursor=(-?\d+)"
        ),
        "sourceCellPresent": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.sourceCellPresent=(-?\d+)")
        ),
        "recordIndex": _search_int(src, rf"{re.escape(name)}\.recordIndex=(-?\d+)"),
        "secondaryRecordIndex": _search_int(
            src, rf"{re.escape(name)}\.secondaryRecordIndex=(-?\d+)"
        ),
        "startDueFrame": _search_int(src, rf"{re.escape(name)}\.startDueFrame=(-?\d+)"),
        "rowMode": _search_int(src, rf"{re.escape(name)}\.rowMode=(-?\d+)"),
        "primaryHead0": _search_int(src, rf"{re.escape(name)}\.primaryHead0=(-?\d+)"),
        "primaryHead1": _search_int(src, rf"{re.escape(name)}\.primaryHead1=(-?\d+)"),
        "streamAAllZero": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.streamAAllZero=(-?\d+)")
        ),
        "streamBAllZero": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.streamBAllZero=(-?\d+)")
        ),
        "streamAHasVisibleBodyPayload": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.streamAHasVisibleBodyPayload=(-?\d+)")
        ),
        "currentCursor": _search_int(src, rf"{re.escape(name)}\.currentCursor=(-?\d+)"),
        "currentEnable": _search_int(src, rf"{re.escape(name)}\.currentEnable=(-?\d+)"),
        "effectiveCursor": _search_int(src, rf"{re.escape(name)}\.effectiveCursor=(-?\d+)"),
        "portraitVisible": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.portraitVisible=(-?\d+)")
        ),
        "rollFrame": _search_int(src, rf"{re.escape(name)}\.rollFrame=(-?\d+)"),
        "streamA": _parse_identity_stream(src, f"{name}.streamA[18]"),
        "streamB": _parse_identity_stream(src, f"{name}.streamB[18]"),
    }


def _parse_local_teacher_icon_sequence_probe(src: str, name: str) -> dict[str, Any]:
    return {
        "available": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.available=(-?\d+)")),
        "sourceEventPsxAddr": _search_hex(
            src,
            rf"{re.escape(name)}\.sourceEventPsxAddr=0x([0-9A-Fa-f]+)",
        ),
        "sourceDueFrame": _search_int(src, rf"{re.escape(name)}\.sourceDueFrame=(-?\d+)"),
        "stableFamilyEventPsxAddr": _search_hex(
            src,
            rf"{re.escape(name)}\.stableFamilyEventPsxAddr=0x([0-9A-Fa-f]+)",
        ),
        "stableFamilyStartDueFrame": _search_int(
            src,
            rf"{re.escape(name)}\.stableFamilyStartDueFrame=(-?\d+)",
        ),
        "rowIndex": _search_int(src, rf"{re.escape(name)}\.rowIndex=(-?\d+)"),
        "selectorAvailable": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.selectorAvailable=(-?\d+)")
        ),
        "selectorByte0": _search_int(src, rf"{re.escape(name)}\.selectorByte0=(-?\d+)"),
        "selectorByte1": _search_int(src, rf"{re.escape(name)}\.selectorByte1=(-?\d+)"),
        "classToken": _search_int(src, rf"{re.escape(name)}\.classToken=(-?\d+)"),
        "symbolType": _search_int(src, rf"{re.escape(name)}\.symbolType=(-?\d+)"),
        "sourceCellId": _search_hex(src, rf"{re.escape(name)}\.sourceCellId=0x([0-9A-Fa-f]+)"),
        "sourceCellHeaderPresent": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.sourceCellHeaderPresent=(-?\d+)")
        ),
        "sourceCellHeaderCount": _search_int(
            src, rf"{re.escape(name)}\.sourceCellHeaderCount=(-?\d+)"
        ),
        "sourceCellHeaderCursor": _search_int(
            src, rf"{re.escape(name)}\.sourceCellHeaderCursor=(-?\d+)"
        ),
        "sourceCellPresent": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.sourceCellPresent=(-?\d+)")
        ),
        "recordIndex": _search_int(src, rf"{re.escape(name)}\.recordIndex=(-?\d+)"),
        "secondaryRecordIndex": _search_int(
            src, rf"{re.escape(name)}\.secondaryRecordIndex=(-?\d+)"
        ),
        "startDueFrame": _search_int(src, rf"{re.escape(name)}\.startDueFrame=(-?\d+)"),
        "rowMode": _search_int(src, rf"{re.escape(name)}\.rowMode=(-?\d+)"),
        "primaryHead0": _search_int(src, rf"{re.escape(name)}\.primaryHead0=(-?\d+)"),
        "primaryHead1": _search_int(src, rf"{re.escape(name)}\.primaryHead1=(-?\d+)"),
        "seedCursor": _search_int(src, rf"{re.escape(name)}\.seedCursor=(-?\d+)"),
        "seedEnable": _search_int(src, rf"{re.escape(name)}\.seedEnable=(-?\d+)"),
        "streamAAllZero": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.streamAAllZero=(-?\d+)")
        ),
        "streamBAllZero": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.streamBAllZero=(-?\d+)")
        ),
        "streamAHasVisibleBodyPayload": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.streamAHasVisibleBodyPayload=(-?\d+)")
        ),
        "currentCursor": _search_int(src, rf"{re.escape(name)}\.currentCursor=(-?\d+)"),
        "currentEnable": _search_int(src, rf"{re.escape(name)}\.currentEnable=(-?\d+)"),
        "effectiveCursor": _search_int(src, rf"{re.escape(name)}\.effectiveCursor=(-?\d+)"),
        "portraitVisible": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.portraitVisible=(-?\d+)")
        ),
        "rollFrame": _search_int(src, rf"{re.escape(name)}\.rollFrame=(-?\d+)"),
        "phaseAvailable": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.phaseAvailable=(-?\d+)")
        ),
        "phaseArmed": _int_to_bool(
            _search_int(src, rf"{re.escape(name)}\.phaseArmed=(-?\d+)")
        ),
        "phaseKey": _search_hex(src, rf"{re.escape(name)}\.phaseKey=0x([0-9A-Fa-f]+)"),
        "cadenceGateStartFrame": _search_int(
            src, rf"{re.escape(name)}\.cadenceGateStartFrame=(-?\d+)"
        ),
        "cadenceSeedStartDueFrame": _search_int(
            src, rf"{re.escape(name)}\.cadenceSeedStartDueFrame=(-?\d+)"
        ),
        "cadenceStartFrameUsed": _search_int(
            src, rf"{re.escape(name)}\.cadenceStartFrameUsed=(-?\d+)"
        ),
        "phaseFirstValidQueryFrame": _search_int(
            src, rf"{re.escape(name)}\.phaseFirstValidQueryFrame=(-?\d+)"
        ),
        "phaseFirstVisibleQueryFrame": _search_int(
            src, rf"{re.escape(name)}\.phaseFirstVisibleQueryFrame=(-?\d+)"
        ),
        "streamA": _parse_identity_stream(src, f"{name}.streamA[18]"),
        "streamB": _parse_identity_stream(src, f"{name}.streamB[18]"),
    }


def _parse_runtime_pair_slot(src: str, name: str) -> dict[str, Any]:
    return {
        "valid": _int_to_bool(_search_int(src, rf"{re.escape(name)}\.valid=(-?\d+)")),
        "pairIndex": _search_int(src, rf"{re.escape(name)}\.pairIndex=(-?\d+)"),
        "startFrame": _search_int(src, rf"{re.escape(name)}\.startFrame=(-?\d+)"),
        "sourceEventPsxAddr": _search_hex(src, rf"{re.escape(name)}\.sourceEventPsxAddr=0x([0-9A-Fa-f]+)"),
        "datHandle": _search_int(src, rf"{re.escape(name)}\.datHandle=(-?\d+)"),
        "vdfHandle": _search_int(src, rf"{re.escape(name)}\.vdfHandle=(-?\d+)"),
    }


def _parse_overlay_render(src: str) -> dict[str, Any]:
    overlay = {
        "nativeRailActivationContent": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailActivationContent=(-?\d+)")
        ),
        "nativeRailVisible": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailVisible=(-?\d+)")
        ),
        "nativeRailRendered": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailRendered=(-?\d+)")
        ),
        "nativeRailTeacherDraw": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailTeacherDraw=(-?\d+)")
        ),
        "nativeRailTeacherPortraitDraw": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailTeacherPortraitDraw=(-?\d+)")
        ),
        "nativeRailStudentDraw": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailStudentDraw=(-?\d+)")
        ),
        "nativeRailStudentPortraitDraw": _int_to_bool(
            _search_int(src, r"overlayRender\.nativeRailStudentPortraitDraw=(-?\d+)")
        ),
        "nativeRailSymbolCount": _search_int(
            src, r"overlayRender\.nativeRailSymbolCount=(-?\d+)"
        ),
        "nativeRailRenderedBodyCount": _search_int(
            src, r"overlayRender\.nativeRailRenderedBodyCount=(-?\d+)"
        ),
    }
    return overlay


def parse_stage1ovl_rail(raw: str) -> dict[str, Any]:
    src = (raw or "").strip()
    result = {
        "command_ok": bool(src) and not src.startswith("ERROR:") and not src.startswith("ERR"),
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
            "currentCommittedDescriptor": _parse_descriptor("", "currentCommittedDescriptor"),
            "nextLookaheadCue": {"available": None, "symbolType": None},
            "nextLookaheadDescriptor": _parse_descriptor("", "nextLookaheadDescriptor"),
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
            "producer": {
                "active": None,
                "activeStreamId": None,
                "lastAppliedEventPsxAddr": None,
                "lastAppliedEventDueFrame": None,
                "activeEventAvailable": None,
                "activeEventDueFrame": None,
                "streamBaseDueFrame": None,
                "activeEventIndex": None,
                "recordIndex": None,
                "secondaryRecordIndex": None,
                "rowMode": None,
                "teacherSource": {"name": None, "value": None},
                "streamA": None,
                "streamB": None,
                "teacherCandidate": _parse_family_candidate("", "cursor.producer.teacherCandidate"),
                "latestTeacherFamily": _parse_family_candidate("", "cursor.producer.latestTeacherFamily"),
                "nextTeacherPreviewFamily": _parse_family_candidate("", "cursor.producer.nextTeacherPreviewFamily"),
                "teacherPreviewShadow": _parse_preview_shadow("", "cursor.producer.teacherPreviewShadow"),
                "teacherFormalCurrentLaneOwnershipReady": None,
                "teacherFormalCurrentLaneVisibleAnchorAheadOfPendingPreview": None,
                "teacherFormalPreviewSequenceFuturePromotable": None,
                "teacherFormalCurrentLaneWaitingForStudentTailTargetWindow": None,
                "teacherFormalShouldHoldPreviewOwnership": None,
                "teacherFormalShouldHoldCurrentLaneOwnership": None,
                "teacherFormalShouldPreferLiveSequenceAuthority": None,
            },
            "teacher": _parse_actor("", "teacher"),
            "student": _parse_actor("", "student"),
            "studentBodyPreview": _parse_actor("", "studentBodyPreview"),
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
        "runtimeSlots": {
            "valid": None,
            "queryFrame": None,
            "mode": None,
            "onMotion": _parse_runtime_pair_slot("", "runtimeSlots.onMotion"),
            "paMotion": _parse_runtime_pair_slot("", "runtimeSlots.paMotion"),
        },
        "nativeRailConsumer": {
            "valid": None,
            "queryFrame": None,
            "queryFrame60": None,
            "teacherOwnedVisualPhase": None,
            "teacherCompactValueRaw": None,
            "studentCompactValueRaw": None,
            "teacherCompactPhaseAvailable": None,
            "teacherCompactCurrentEnable": None,
            "teacherCompactPhaseKey": None,
            "teacherCompactCadenceStartFrameUsed": None,
            "teacherLargeMarkersOn": None,
            "teacherSmallMarkersOn": None,
            "teacherPortrait": {
                "livePortraitCursorRaw": None,
                "effectiveCursor": None,
                "renderPortraitSampleCursorRaw": None,
                "renderPortraitSampleVisible": None,
            },
            "studentPortrait": {
                "livePortraitCursorRaw": None,
                "effectiveCursor": None,
                "renderPortraitSampleCursorRaw": None,
                "renderPortraitSampleVisible": None,
            },
        },
        "overlayRender": _parse_overlay_render(""),
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

    result["runtime_available"] = True
    rail = result["rail"]
    cursor = result["cursor"]
    producer = cursor["producer"]
    display_gate = result["displayGate"]
    runtime_slots = result["runtimeSlots"]
    native_rail_consumer = result["nativeRailConsumer"]
    handoff = re.search(r"rail\.acceptedHandoffPhase=([A-Za-z_]+)\((-?\d+)\)", src)

    rail.update(
        {
            "valid": _int_to_bool(_search_int(src, r"rail\.valid=(-?\d+)")),
            "queryFrame": _search_int(src, r"rail\.queryFrame=(-?\d+)"),
            "rowMode": _search_int(src, r"rail\.rowMode=(-?\d+)"),
            "bodyStreamA": _search_int_list(src, r"rail\.bodyStreamA\[18\]=\[(.*?)\]"),
            "currentCommittedCue": _parse_cue(src, "currentCommittedCue"),
            "currentCommittedDescriptor": _parse_descriptor(src, "currentCommittedDescriptor"),
            "nextLookaheadCue": _parse_cue(src, "nextLookaheadCue"),
            "nextLookaheadCueWindowActive": _int_to_bool(
                _search_int(src, r"rail\.nextLookaheadCueWindowActive=(-?\d+)")
            ),
            "nextLookaheadAcceptedContributionEnabled": _int_to_bool(
                _search_int(src, r"rail\.nextLookaheadAcceptedContributionEnabled=(-?\d+)")
            ),
            "nextLookaheadDescriptor": _parse_descriptor(src, "nextLookaheadDescriptor"),
            "acceptedBoundaryCue": _parse_cue(src, "acceptedBoundaryCue"),
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
            "teacher": _parse_actor(src, "teacher"),
            "student": _parse_actor(src, "student"),
            "studentBodyPreview": _parse_actor(src, "studentBodyPreview"),
        }
    )
    producer.update(
        {
            "active": _int_to_bool(_search_int(src, r"cursor\.producer\.active=(-?\d+)")),
            "activeStreamId": _search_int(src, r"cursor\.producer\.activeStreamId=(-?\d+)"),
            "lastAppliedEventPsxAddr": _search_hex(
                src,
                r"cursor\.producer\.lastAppliedEventPsxAddr=0x([0-9A-Fa-f]+)",
            ),
            "lastAppliedEventDueFrame": _search_int(
                src,
                r"cursor\.producer\.lastAppliedEventDueFrame=(-?\d+)",
            ),
            "activeEventAvailable": _int_to_bool(
                _search_int(src, r"cursor\.producer\.activeEventAvailable=(-?\d+)")
            ),
            "activeEventDueFrame": _search_int(src, r"cursor\.producer\.activeEventDueFrame=(-?\d+)"),
            "streamBaseDueFrame": _search_int(src, r"cursor\.producer\.streamBaseDueFrame=(-?\d+)"),
            "activeEventIndex": _search_int(src, r"cursor\.producer\.activeEventIndex=(-?\d+)"),
            "recordIndex": _search_int(src, r"cursor\.producer\.recordIndex=(-?\d+)"),
            "secondaryRecordIndex": _search_int(
                src,
                r"cursor\.producer\.secondaryRecordIndex=(-?\d+)",
            ),
            "rowMode": _search_int(src, r"cursor\.producer\.rowMode=(-?\d+)"),
            "teacherSource": _parse_named_enum(src, "cursor.producer.teacherSource"),
            "teacherFormalAuthorityApplied": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalAuthorityApplied=(-?\d+)",
                )
            ),
            "teacherFormalAuthorityOwnsTeacher": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalAuthorityOwnsTeacher=(-?\d+)",
                )
            ),
            "teacherTransitionFallbackApplied": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherTransitionFallbackApplied=(-?\d+)",
                )
            ),
            "teacherFormalCurrentLaneOwnershipReady": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalCurrentLaneOwnershipReady=(-?\d+)",
                )
            ),
            "teacherFormalCurrentLaneVisibleAnchorAheadOfPendingPreview": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalCurrentLaneVisibleAnchorAheadOfPendingPreview=(-?\d+)",
                )
            ),
            "teacherFormalPreviewSequenceFuturePromotable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalPreviewSequenceFuturePromotable=(-?\d+)",
                )
            ),
            "teacherFormalCurrentLaneWaitingForStudentTailTargetWindow": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalCurrentLaneWaitingForStudentTailTargetWindow=(-?\d+)",
                )
            ),
            "teacherFormalShouldHoldPreviewOwnership": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalShouldHoldPreviewOwnership=(-?\d+)",
                )
            ),
            "teacherFormalShouldHoldCurrentLaneOwnership": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalShouldHoldCurrentLaneOwnership=(-?\d+)",
                )
            ),
            "teacherFormalShouldPreferLiveSequenceAuthority": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherFormalShouldPreferLiveSequenceAuthority=(-?\d+)",
                )
            ),
            "teacherCurrentLaneTailTransitionFallbackSuppressed": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherCurrentLaneTailTransitionFallbackSuppressed=(-?\d+)",
                )
            ),
            "teacherPendingPreviewTransitionFallbackSuppressed": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.teacherPendingPreviewTransitionFallbackSuppressed=(-?\d+)",
                )
            ),
            "psxPromotedLiveKind": _search_int(
                src, r"cursor\.producer\.psxPromotedLiveKind=(-?\d+)"
            ),
            "psxLiveTimingSelectedSourceKind": _search_int(
                src, r"cursor\.producer\.psxLiveTimingSelectedSourceKind=(-?\d+)"
            ),
            "psxLiveTimingSelectedStartDueFrame": _search_int(
                src,
                r"cursor\.producer\.psxLiveTimingSelectedStartDueFrame=(-?\d+)",
            ),
            "psxLiveTimingLocalPreviewCandidateAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveTimingLocalPreviewCandidateAvailable=(-?\d+)",
                )
            ),
            "psxLiveTimingLocalPreviewCandidateFuturePromotable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveTimingLocalPreviewCandidateFuturePromotable=(-?\d+)",
                )
            ),
            "psxLiveTimingLocalPreviewCandidateSourceDueFrameAheadOfSelectedSource": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveTimingLocalPreviewCandidateSourceDueFrameAheadOfSelectedSource=(-?\d+)",
                )
            ),
            "psxLiveTimingLocalPreviewCandidatePromoteNow": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveTimingLocalPreviewCandidatePromoteNow=(-?\d+)",
                )
            ),
            "psxLiveSequenceSelectedSourceKind": _search_int(
                src, r"cursor\.producer\.psxLiveSequenceSelectedSourceKind=(-?\d+)"
            ),
            "psxLiveSequencePreviewCandidateAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequencePreviewCandidateAvailable=(-?\d+)",
                )
            ),
            "psxLiveSequencePreviewCandidateFuturePromotable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequencePreviewCandidateFuturePromotable=(-?\d+)",
                )
            ),
            "psxLiveSequencePreviewCandidatePromoteNow": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequencePreviewCandidatePromoteNow=(-?\d+)",
                )
            ),
            "psxLiveSequencePreviewCandidatePayloadFromLocalSequence": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequencePreviewCandidatePayloadFromLocalSequence=(-?\d+)",
                )
            ),
            "psxLiveSequencePreviewCandidatePayloadFromPreviousLocalSequence": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequencePreviewCandidatePayloadFromPreviousLocalSequence=(-?\d+)",
                )
            ),
            "psxLiveSequencePreviewCandidateSourceDueFrameMatchesTimingAnchor": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequencePreviewCandidateSourceDueFrameMatchesTimingAnchor=(-?\d+)",
                )
            ),
            "psxLiveSequenceCurrentLaneCandidateAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequenceCurrentLaneCandidateAvailable=(-?\d+)",
                )
                or _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequenceLocalCandidateAvailable=(-?\d+)",
                )
            ),
            "psxLiveSequenceCurrentLaneCandidatePromoteNow": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequenceCurrentLaneCandidatePromoteNow=(-?\d+)",
                )
                or _search_int(
                    src,
                    r"cursor\.producer\.psxLiveSequenceLocalCandidatePromoteNow=(-?\d+)",
                )
            ),
            "psxLiveCadenceSelectedKind": _search_int(
                src, r"cursor\.producer\.psxLiveCadenceSelectedKind=(-?\d+)"
            ),
            "psxLiveCadenceSelectedStartFrame": _search_int(
                src,
                r"cursor\.producer\.psxLiveCadenceSelectedStartFrame=(-?\d+)",
            ),
            "studentLiveSelectedSourceKind": _search_int(
                src, r"cursor\.producer\.studentLiveSelectedSourceKind=(-?\d+)"
            ),
            "studentLiveAuthoritySelectedSourceKind": _search_int(
                src,
                r"cursor\.producer\.studentLiveAuthoritySelectedSourceKind=(-?\d+)",
            ),
            "studentCurrentLaneBodyPreviewAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentLaneBodyPreviewAvailable=(-?\d+)",
                )
            ),
            "studentLiveTimingSelectedSourceKind": _search_int(
                src, r"cursor\.producer\.studentLiveTimingSelectedSourceKind=(-?\d+)"
            ),
            "studentCurrentLaneTimingCandidateAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentLaneTimingCandidateAvailable=(-?\d+)",
                )
            ),
            "studentCurrentLaneTimingCandidateDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentCurrentLaneTimingCandidateDueFrame=(-?\d+)",
            ),
            "studentLiveTimingSelectedStartDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentLiveTimingSelectedStartDueFrame=(-?\d+)",
            ),
            "studentLiveTimingSelectedCadenceStartFrame": _search_int(
                src,
                r"cursor\.producer\.studentLiveTimingSelectedCadenceStartFrame=(-?\d+)",
            ),
            "studentCurrentLaneTimingAnchorArmed": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentLaneTimingAnchorArmed=(-?\d+)",
                )
            ),
            "studentCurrentLaneTimingAnchorStartFrame": _search_int(
                src,
                r"cursor\.producer\.studentCurrentLaneTimingAnchorStartFrame=(-?\d+)",
            ),
            "studentCurrentLaneTimingAnchorPhaseKey": _search_hex(
                src,
                r"cursor\.producer\.studentCurrentLaneTimingAnchorPhaseKey=0x([0-9A-Fa-f]+)",
            ),
            "studentCurrentLaneContinuationActive": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentLaneContinuationActive=(-?\d+)",
                )
            ),
            "studentCurrentLaneCandidateReady": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentLaneCandidateReady=(-?\d+)",
                )
            ),
            "studentCurrentLaneCandidateEffectiveCursor": _search_int(
                src,
                r"cursor\.producer\.studentCurrentLaneCandidateEffectiveCursor=(-?\d+)",
            ),
            "studentPreviewFuturePromotableInput": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewFuturePromotableInput=(-?\d+)",
                )
            ),
            "studentPreviewSeedSourceKind": _search_int(
                src, r"cursor\.producer\.studentPreviewSeedSourceKind=(-?\d+)"
            ),
            "studentAllowPreviewProbeBootstrap": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentAllowPreviewProbeBootstrap=(-?\d+)",
                )
            ),
            "studentPreviewProbeFutureSourceActive": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeFutureSourceActive=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapTeacherOwnedFamilyBranch": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapTeacherOwnedFamilyBranch=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapSameFamilyLateTailWindow": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapSameFamilyLateTailWindow=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapFutureSourceProbeActive": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapFutureSourceProbeActive=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapTeacherMaterializedLaterSameSourceFamily": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapTeacherMaterializedLaterSameSourceFamily=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapFamilyStartDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentPreviewProbeBootstrapFamilyStartDueFrame=(-?\d+)",
            ),
            "studentPreviewProbeBootstrapQueryReachedProbeStart": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapQueryReachedProbeStart=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapTeacherAdvancedToLaterPreviewFamily": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapTeacherAdvancedToLaterPreviewFamily=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapPreviousStudentFamilyFinished": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapPreviousStudentFamilyFinished=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapRememberedStudentFamilyFinished": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapRememberedStudentFamilyFinished=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapFinishedFutureProbeBootstrap": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapFinishedFutureProbeBootstrap=(-?\d+)",
                )
            ),
            "studentPreviewProbeBootstrapRejectedBySupersedingPreviousBootstrapFamily": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentPreviewProbeBootstrapRejectedBySupersedingPreviousBootstrapFamily=(-?\d+)",
                )
            ),
            "studentLatestSeedHasVisibleBody": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentLatestSeedHasVisibleBody=(-?\d+)",
                )
            ),
            "studentLatestSeedFamilyDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentLatestSeedFamilyDueFrame=(-?\d+)",
            ),
            "studentNextSeedFamilyDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentNextSeedFamilyDueFrame=(-?\d+)",
            ),
            "studentPersistedPreviewFamilyDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentPersistedPreviewFamilyDueFrame=(-?\d+)",
            ),
            "studentPreviewSeedFamilyDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentPreviewSeedFamilyDueFrame=(-?\d+)",
            ),
            "studentCurrentTeacherPreviewProbeAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentTeacherPreviewProbeAvailable=(-?\d+)",
                )
            ),
            "studentCurrentTeacherPreviewProbeSourceDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentCurrentTeacherPreviewProbeSourceDueFrame=(-?\d+)",
            ),
            "studentCurrentTeacherPreviewProbeStartDueFrame": _search_int(
                src,
                r"cursor\.producer\.studentCurrentTeacherPreviewProbeStartDueFrame=(-?\d+)",
            ),
            "studentCurrentTeacherPreviewProbeVisibleBodyPayload": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentTeacherPreviewProbeVisibleBodyPayload=(-?\d+)",
                )
            ),
            "studentCurrentLaneMaterializationAllowed": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentCurrentLaneMaterializationAllowed=(-?\d+)",
                )
            ),
            "studentTeacherTailOverlapTimingCandidateAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentTeacherTailOverlapTimingCandidateAvailable=(-?\d+)",
                )
            ),
            "studentTeacherTailOverlapTimingCandidateStartFrame": _search_int(
                src,
                r"cursor\.producer\.studentTeacherTailOverlapTimingCandidateStartFrame=(-?\d+)",
            ),
            "studentTeacherTailOverlapTimingAnchorArmed": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentTeacherTailOverlapTimingAnchorArmed=(-?\d+)",
                )
            ),
            "studentTeacherTailOverlapTimingAnchorStartFrame": _search_int(
                src,
                r"cursor\.producer\.studentTeacherTailOverlapTimingAnchorStartFrame=(-?\d+)",
            ),
            "studentTeacherTailOverlapTimingAnchorPhaseKey": _search_hex(
                src,
                r"cursor\.producer\.studentTeacherTailOverlapTimingAnchorPhaseKey=0x([0-9A-Fa-f]+)",
            ),
            "studentTeacherTailTargetWindowObserved": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentTeacherTailTargetWindowObserved=(-?\d+)",
                )
            ),
            "studentTeacherTailTargetWindowFirstQueryFrame": _search_int(
                src,
                r"cursor\.producer\.studentTeacherTailTargetWindowFirstQueryFrame=(-?\d+)",
            ),
            "studentLiveSequenceSelectedSourceKind": _search_int(
                src, r"cursor\.producer\.studentLiveSequenceSelectedSourceKind=(-?\d+)"
            ),
            "studentLiveApplyBlockOrdinal": _search_int(
                src, r"cursor\.producer\.studentLiveApplyBlockOrdinal=(-?\d+)"
            ),
            "studentLiveApplyAuthorityAvailable": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentLiveApplyAuthorityAvailable=(-?\d+)",
                )
            ),
            "studentLiveApplyMaterializationAllowed": _int_to_bool(
                _search_int(
                    src,
                    r"cursor\.producer\.studentLiveApplyMaterializationAllowed=(-?\d+)",
                )
            ),
            "studentCadenceGateArmed": _int_to_bool(
                _search_int(src, r"cursor\.producer\.studentCadenceGateArmed=(-?\d+)")
            ),
            "studentCadenceGateStartFrame": _search_int(
                src, r"cursor\.producer\.studentCadenceGateStartFrame=(-?\d+)"
            ),
            "studentCadenceGatePhaseKey": _search_hex(
                src, r"cursor\.producer\.studentCadenceGatePhaseKey=0x([0-9A-Fa-f]+)"
            ),
            "streamA": _parse_identity_stream(src, "cursor.producer.streamA[18]"),
            "streamB": _parse_identity_stream(src, "cursor.producer.streamB[18]"),
            "teacherCandidate": _parse_family_candidate(src, "cursor.producer.teacherCandidate"),
            "latestTeacherFamily": _parse_family_candidate(src, "cursor.producer.latestTeacherFamily"),
            "nextTeacherPreviewFamily": _parse_family_candidate(
                src,
                "cursor.producer.nextTeacherPreviewFamily",
            ),
            "teacherPreviewShadow": _parse_preview_shadow(src, "cursor.producer.teacherPreviewShadow"),
            "localTeacherPreviewProbe": _parse_local_teacher_preview_probe(
                src, "cursor.producer.localTeacherPreviewProbe"
            ),
            "localTeacherIconSequenceProbe": _parse_local_teacher_icon_sequence_probe(
                src, "cursor.producer.localTeacherIconSequenceProbe"
            ),
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

    runtime_slots.update(
        {
            "valid": _int_to_bool(_search_int(src, r"runtimeSlots\.valid=(-?\d+)")),
            "queryFrame": _search_int(src, r"runtimeSlots\.queryFrame=(-?\d+)"),
            "mode": _search_int(src, r"runtimeSlots\.mode=(-?\d+)"),
            "onMotion": _parse_runtime_pair_slot(src, "runtimeSlots.onMotion"),
            "paMotion": _parse_runtime_pair_slot(src, "runtimeSlots.paMotion"),
        }
    )
    def _search_int_any(patterns: List[str]) -> Optional[int]:
        for pattern in patterns:
            value = _search_int(src, pattern)
            if value is not None:
                return value
        return None

    def _search_bool_any(patterns: List[str]) -> Optional[bool]:
        value = _search_int_any(patterns)
        return _int_to_bool(value)

    native_rail_consumer.update(
        {
            "valid": _int_to_bool(
                _search_int(src, r"nativeRailConsumer\.valid=(-?\d+)")
            ),
            "queryFrame": _search_int(
                src, r"nativeRailConsumer\.queryFrame=(-?\d+)"
            ),
            "queryFrame60": _search_int(
                src, r"nativeRailConsumer\.queryFrame60=(-?\d+)"
            ),
            "teacherOwnedVisualPhase": _int_to_bool(
                _search_int(
                    src, r"nativeRailConsumer\.teacherOwnedVisualPhase=(-?\d+)"
                )
            ),
            "teacherCompactValueRaw": _search_int(
                src, r"nativeRailConsumer\.teacherCompactValueRaw=(-?\d+)"
            ),
            "studentCompactValueRaw": _search_int(
                src, r"nativeRailConsumer\.studentCompactValueRaw=(-?\d+)"
            ),
            "slot0CompactValueRaw": _search_int_any(
                [
                    r"nativeRailConsumer\.slot0CompactValueRaw=(-?\d+)",
                    r"nativeRailConsumer\.teacherCompactValueRaw=(-?\d+)",
                ]
            ),
            "slot1CompactValueRaw": _search_int_any(
                [
                    r"nativeRailConsumer\.slot1CompactValueRaw=(-?\d+)",
                    r"nativeRailConsumer\.studentCompactValueRaw=(-?\d+)",
                ]
            ),
            "teacherCompactPhaseAvailable": _int_to_bool(
                _search_int(
                    src, r"nativeRailConsumer\.teacherCompactPhaseAvailable=(-?\d+)"
                )
            ),
            "teacherCompactCurrentEnable": _int_to_bool(
                _search_int(
                    src, r"nativeRailConsumer\.teacherCompactCurrentEnable=(-?\d+)"
                )
            ),
            "teacherCompactPhaseKey": _search_int(
                src, r"nativeRailConsumer\.teacherCompactPhaseKey=(-?\d+)"
            ),
            "teacherCompactCadenceStartFrameUsed": _search_int(
                src,
                r"nativeRailConsumer\.teacherCompactCadenceStartFrameUsed=(-?\d+)",
            ),
            "slot0CompactPhaseAvailable": _search_bool_any(
                [
                    r"nativeRailConsumer\.slot0CompactPhaseAvailable=(-?\d+)",
                    r"nativeRailConsumer\.teacherCompactPhaseAvailable=(-?\d+)",
                ]
            ),
            "slot0CompactCurrentEnable": _search_bool_any(
                [
                    r"nativeRailConsumer\.slot0CompactCurrentEnable=(-?\d+)",
                    r"nativeRailConsumer\.teacherCompactCurrentEnable=(-?\d+)",
                ]
            ),
            "slot0CompactPhaseKey": _search_int_any(
                [
                    r"nativeRailConsumer\.slot0CompactPhaseKey=(-?\d+)",
                    r"nativeRailConsumer\.teacherCompactPhaseKey=(-?\d+)",
                ]
            ),
            "slot0CompactCadenceStartFrameUsed": _search_int_any(
                [
                    r"nativeRailConsumer\.slot0CompactCadenceStartFrameUsed=(-?\d+)",
                    r"nativeRailConsumer\.teacherCompactCadenceStartFrameUsed=(-?\d+)",
                ]
            ),
            "teacherLargeMarkersOn": _search_int_list(
                src, r"nativeRailConsumer\.teacherLargeMarkersOn\[4\]=\[(.*?)\]"
            ),
            "teacherSmallMarkersOn": _search_int_list(
                src, r"nativeRailConsumer\.teacherSmallMarkersOn\[14\]=\[(.*?)\]"
            ),
            "teacherPortrait": {
                "livePortraitCursorRaw": _search_int(
                    src,
                    r"nativeRailConsumer\.teacherPortrait\.livePortraitCursorRaw=(-?\d+)",
                ),
                "family": _search_int(
                    src,
                    r"nativeRailConsumer\.teacherPortrait\.family=(-?\d+)",
                ),
                "effectiveCursor": _search_int(
                    src,
                    r"nativeRailConsumer\.teacherPortrait\.effectiveCursor=(-?\d+)",
                ),
                "renderPortraitSampleCursorRaw": _search_int(
                    src,
                    r"nativeRailConsumer\.teacherPortrait\.renderPortraitSampleCursorRaw=(-?\d+)",
                ),
                "renderPortraitSampleVisible": _int_to_bool(
                    _search_int(
                        src,
                        r"nativeRailConsumer\.teacherPortrait\.renderPortraitSampleVisible=(-?\d+)",
                    )
                ),
            },
            "slot0Portrait": {
                "livePortraitCursorRaw": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot0Portrait\.livePortraitCursorRaw=(-?\d+)",
                        r"nativeRailConsumer\.teacherPortrait\.livePortraitCursorRaw=(-?\d+)",
                    ]
                ),
                "family": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot0Portrait\.family=(-?\d+)",
                        r"nativeRailConsumer\.teacherPortrait\.family=(-?\d+)",
                    ]
                ),
                "effectiveCursor": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot0Portrait\.effectiveCursor=(-?\d+)",
                        r"nativeRailConsumer\.teacherPortrait\.effectiveCursor=(-?\d+)",
                    ]
                ),
                "renderPortraitSampleCursorRaw": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot0Portrait\.renderPortraitSampleCursorRaw=(-?\d+)",
                        r"nativeRailConsumer\.teacherPortrait\.renderPortraitSampleCursorRaw=(-?\d+)",
                    ]
                ),
                "renderPortraitSampleVisible": _search_bool_any(
                    [
                        r"nativeRailConsumer\.slot0Portrait\.renderPortraitSampleVisible=(-?\d+)",
                        r"nativeRailConsumer\.teacherPortrait\.renderPortraitSampleVisible=(-?\d+)",
                    ]
                ),
            },
            "studentPortrait": {
                "livePortraitCursorRaw": _search_int(
                    src,
                    r"nativeRailConsumer\.studentPortrait\.livePortraitCursorRaw=(-?\d+)",
                ),
                "family": _search_int(
                    src,
                    r"nativeRailConsumer\.studentPortrait\.family=(-?\d+)",
                ),
                "effectiveCursor": _search_int(
                    src,
                    r"nativeRailConsumer\.studentPortrait\.effectiveCursor=(-?\d+)",
                ),
                "renderPortraitSampleCursorRaw": _search_int(
                    src,
                    r"nativeRailConsumer\.studentPortrait\.renderPortraitSampleCursorRaw=(-?\d+)",
                ),
                "renderPortraitSampleVisible": _int_to_bool(
                    _search_int(
                        src,
                        r"nativeRailConsumer\.studentPortrait\.renderPortraitSampleVisible=(-?\d+)",
                    )
                ),
            },
            "slot1Portrait": {
                "livePortraitCursorRaw": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot1Portrait\.livePortraitCursorRaw=(-?\d+)",
                        r"nativeRailConsumer\.studentPortrait\.livePortraitCursorRaw=(-?\d+)",
                    ]
                ),
                "family": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot1Portrait\.family=(-?\d+)",
                        r"nativeRailConsumer\.studentPortrait\.family=(-?\d+)",
                    ]
                ),
                "effectiveCursor": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot1Portrait\.effectiveCursor=(-?\d+)",
                        r"nativeRailConsumer\.studentPortrait\.effectiveCursor=(-?\d+)",
                    ]
                ),
                "renderPortraitSampleCursorRaw": _search_int_any(
                    [
                        r"nativeRailConsumer\.slot1Portrait\.renderPortraitSampleCursorRaw=(-?\d+)",
                        r"nativeRailConsumer\.studentPortrait\.renderPortraitSampleCursorRaw=(-?\d+)",
                    ]
                ),
                "renderPortraitSampleVisible": _search_bool_any(
                    [
                        r"nativeRailConsumer\.slot1Portrait\.renderPortraitSampleVisible=(-?\d+)",
                        r"nativeRailConsumer\.studentPortrait\.renderPortraitSampleVisible=(-?\d+)",
                    ]
                ),
            },
        }
    )
    result["overlayRender"] = _parse_overlay_render(src)

    essential = (
        rail["queryFrame"],
        rail["rowMode"],
        rail["bodyStreamA"],
        cursor["queryFrame"],
        display_gate["queryFrame"],
        runtime_slots["queryFrame"],
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
        "transState",
        "transStateDA",
    )
    return {name: status.get(name) for name in fields}


def _make_focus(parsed: dict[str, Any]) -> dict[str, Any]:
    rail = parsed.get("rail") or {}
    cursor = parsed.get("cursor") or {}
    producer = cursor.get("producer") or {}
    display_gate = parsed.get("displayGate") or {}
    runtime_slots = parsed.get("runtimeSlots") or {}
    native_rail_consumer = parsed.get("nativeRailConsumer") or {}
    overlay_render = parsed.get("overlayRender") or {}
    return {
        "rowMode": rail.get("rowMode"),
        "lessonSelector": display_gate.get("lessonSelector"),
        "bodyStreamA": rail.get("bodyStreamA"),
        "currentCommittedCue": rail.get("currentCommittedCue"),
        "currentCommittedDescriptor": rail.get("currentCommittedDescriptor"),
        "nextLookaheadCue": rail.get("nextLookaheadCue"),
        "nextLookaheadDescriptor": rail.get("nextLookaheadDescriptor"),
        "acceptedBoundaryCue": rail.get("acceptedBoundaryCue"),
        "displayGate": display_gate,
        "producer": {
            "teacherSource": producer.get("teacherSource"),
            "teacherFormalAuthorityApplied": producer.get(
                "teacherFormalAuthorityApplied"
            ),
            "teacherFormalAuthorityOwnsTeacher": producer.get(
                "teacherFormalAuthorityOwnsTeacher"
            ),
            "teacherTransitionFallbackApplied": producer.get(
                "teacherTransitionFallbackApplied"
            ),
            "teacherFormalCurrentLaneOwnershipReady": producer.get(
                "teacherFormalCurrentLaneOwnershipReady"
            ),
            "teacherFormalCurrentLaneVisibleAnchorAheadOfPendingPreview": producer.get(
                "teacherFormalCurrentLaneVisibleAnchorAheadOfPendingPreview"
            ),
            "teacherFormalPreviewSequenceFuturePromotable": producer.get(
                "teacherFormalPreviewSequenceFuturePromotable"
            ),
            "teacherFormalCurrentLaneWaitingForStudentTailTargetWindow": producer.get(
                "teacherFormalCurrentLaneWaitingForStudentTailTargetWindow"
            ),
            "teacherFormalShouldHoldPreviewOwnership": producer.get(
                "teacherFormalShouldHoldPreviewOwnership"
            ),
            "teacherFormalShouldHoldCurrentLaneOwnership": producer.get(
                "teacherFormalShouldHoldCurrentLaneOwnership"
            ),
            "teacherFormalShouldPreferLiveSequenceAuthority": producer.get(
                "teacherFormalShouldPreferLiveSequenceAuthority"
            ),
            "teacherCurrentLaneTailTransitionFallbackSuppressed": producer.get(
                "teacherCurrentLaneTailTransitionFallbackSuppressed"
            ),
            "teacherPendingPreviewTransitionFallbackSuppressed": producer.get(
                "teacherPendingPreviewTransitionFallbackSuppressed"
            ),
            "teacherCandidate": producer.get("teacherCandidate"),
            "latestTeacherFamily": producer.get("latestTeacherFamily"),
            "nextTeacherPreviewFamily": producer.get("nextTeacherPreviewFamily"),
            "teacherPreviewShadow": producer.get("teacherPreviewShadow"),
            "localTeacherPreviewProbe": producer.get("localTeacherPreviewProbe"),
            "localTeacherIconSequenceProbe": producer.get("localTeacherIconSequenceProbe"),
            "psxLiveTimingSelectedSourceKind": producer.get("psxLiveTimingSelectedSourceKind"),
            "psxLiveTimingSelectedStartDueFrame": producer.get(
                "psxLiveTimingSelectedStartDueFrame"
            ),
            "psxLiveTimingLocalPreviewCandidateAvailable": producer.get(
                "psxLiveTimingLocalPreviewCandidateAvailable"
            ),
            "psxLiveTimingLocalPreviewCandidateFuturePromotable": producer.get(
                "psxLiveTimingLocalPreviewCandidateFuturePromotable"
            ),
            "psxLiveTimingLocalPreviewCandidateSourceDueFrameAheadOfSelectedSource": producer.get(
                "psxLiveTimingLocalPreviewCandidateSourceDueFrameAheadOfSelectedSource"
            ),
            "psxLiveTimingLocalPreviewCandidatePromoteNow": producer.get(
                "psxLiveTimingLocalPreviewCandidatePromoteNow"
            ),
            "psxLiveSequenceSelectedSourceKind": producer.get(
                "psxLiveSequenceSelectedSourceKind"
            ),
            "psxLiveSequencePreviewCandidateAvailable": producer.get(
                "psxLiveSequencePreviewCandidateAvailable"
            ),
            "psxLiveSequencePreviewCandidateFuturePromotable": producer.get(
                "psxLiveSequencePreviewCandidateFuturePromotable"
            ),
            "psxLiveSequencePreviewCandidatePromoteNow": producer.get(
                "psxLiveSequencePreviewCandidatePromoteNow"
            ),
            "psxLiveSequencePreviewCandidatePayloadFromLocalSequence": producer.get(
                "psxLiveSequencePreviewCandidatePayloadFromLocalSequence"
            ),
            "psxLiveSequencePreviewCandidatePayloadFromPreviousLocalSequence": producer.get(
                "psxLiveSequencePreviewCandidatePayloadFromPreviousLocalSequence"
            ),
            "psxLiveSequencePreviewCandidateSourceDueFrameMatchesTimingAnchor": producer.get(
                "psxLiveSequencePreviewCandidateSourceDueFrameMatchesTimingAnchor"
            ),
            "psxLiveSequenceCurrentLaneCandidateAvailable": producer.get(
                "psxLiveSequenceCurrentLaneCandidateAvailable"
                if "psxLiveSequenceCurrentLaneCandidateAvailable" in producer
                else "psxLiveSequenceLocalCandidateAvailable"
            ),
            "psxLiveSequenceCurrentLaneCandidatePromoteNow": producer.get(
                "psxLiveSequenceCurrentLaneCandidatePromoteNow"
                if "psxLiveSequenceCurrentLaneCandidatePromoteNow" in producer
                else "psxLiveSequenceLocalCandidatePromoteNow"
            ),
            "psxLiveCadenceSelectedKind": producer.get("psxLiveCadenceSelectedKind"),
            "psxLiveCadenceSelectedStartFrame": producer.get(
                "psxLiveCadenceSelectedStartFrame"
            ),
            "studentLiveSelectedSourceKind": producer.get("studentLiveSelectedSourceKind"),
            "studentLiveAuthoritySelectedSourceKind": producer.get(
                "studentLiveAuthoritySelectedSourceKind"
            ),
            "studentCurrentLaneBodyPreviewAvailable": producer.get(
                "studentCurrentLaneBodyPreviewAvailable"
            ),
            "studentLiveTimingSelectedSourceKind": producer.get(
                "studentLiveTimingSelectedSourceKind"
            ),
            "studentCurrentLaneTimingCandidateAvailable": producer.get(
                "studentCurrentLaneTimingCandidateAvailable"
            ),
            "studentCurrentLaneTimingCandidateDueFrame": producer.get(
                "studentCurrentLaneTimingCandidateDueFrame"
            ),
            "studentLiveTimingSelectedStartDueFrame": producer.get(
                "studentLiveTimingSelectedStartDueFrame"
            ),
            "studentLiveTimingSelectedCadenceStartFrame": producer.get(
                "studentLiveTimingSelectedCadenceStartFrame"
            ),
            "studentCurrentLaneTimingAnchorArmed": producer.get(
                "studentCurrentLaneTimingAnchorArmed"
            ),
            "studentCurrentLaneTimingAnchorStartFrame": producer.get(
                "studentCurrentLaneTimingAnchorStartFrame"
            ),
            "studentCurrentLaneTimingAnchorPhaseKey": producer.get(
                "studentCurrentLaneTimingAnchorPhaseKey"
            ),
            "studentCurrentLaneContinuationActive": producer.get(
                "studentCurrentLaneContinuationActive"
            ),
            "studentCurrentLaneCandidateReady": producer.get(
                "studentCurrentLaneCandidateReady"
            ),
            "studentCurrentLaneCandidateEffectiveCursor": producer.get(
                "studentCurrentLaneCandidateEffectiveCursor"
            ),
            "studentPreviewFuturePromotableInput": producer.get(
                "studentPreviewFuturePromotableInput"
            ),
            "studentPreviewSeedSourceKind": producer.get(
                "studentPreviewSeedSourceKind"
            ),
            "studentAllowPreviewProbeBootstrap": producer.get(
                "studentAllowPreviewProbeBootstrap"
            ),
            "studentPreviewProbeFutureSourceActive": producer.get(
                "studentPreviewProbeFutureSourceActive"
            ),
            "studentPreviewProbeBootstrapTeacherOwnedFamilyBranch": producer.get(
                "studentPreviewProbeBootstrapTeacherOwnedFamilyBranch"
            ),
            "studentPreviewProbeBootstrapSameFamilyLateTailWindow": producer.get(
                "studentPreviewProbeBootstrapSameFamilyLateTailWindow"
            ),
            "studentPreviewProbeBootstrapFutureSourceProbeActive": producer.get(
                "studentPreviewProbeBootstrapFutureSourceProbeActive"
            ),
            "studentPreviewProbeBootstrapTeacherMaterializedLaterSameSourceFamily": producer.get(
                "studentPreviewProbeBootstrapTeacherMaterializedLaterSameSourceFamily"
            ),
            "studentPreviewProbeBootstrapFamilyStartDueFrame": producer.get(
                "studentPreviewProbeBootstrapFamilyStartDueFrame"
            ),
            "studentPreviewProbeBootstrapQueryReachedProbeStart": producer.get(
                "studentPreviewProbeBootstrapQueryReachedProbeStart"
            ),
            "studentPreviewProbeBootstrapTeacherAdvancedToLaterPreviewFamily": producer.get(
                "studentPreviewProbeBootstrapTeacherAdvancedToLaterPreviewFamily"
            ),
            "studentPreviewProbeBootstrapPreviousStudentFamilyFinished": producer.get(
                "studentPreviewProbeBootstrapPreviousStudentFamilyFinished"
            ),
            "studentPreviewProbeBootstrapRememberedStudentFamilyFinished": producer.get(
                "studentPreviewProbeBootstrapRememberedStudentFamilyFinished"
            ),
            "studentPreviewProbeBootstrapFinishedFutureProbeBootstrap": producer.get(
                "studentPreviewProbeBootstrapFinishedFutureProbeBootstrap"
            ),
            "studentPreviewProbeBootstrapRejectedBySupersedingPreviousBootstrapFamily": producer.get(
                "studentPreviewProbeBootstrapRejectedBySupersedingPreviousBootstrapFamily"
            ),
            "studentLatestSeedHasVisibleBody": producer.get(
                "studentLatestSeedHasVisibleBody"
            ),
            "studentLatestSeedFamilyDueFrame": producer.get(
                "studentLatestSeedFamilyDueFrame"
            ),
            "studentNextSeedFamilyDueFrame": producer.get(
                "studentNextSeedFamilyDueFrame"
            ),
            "studentPersistedPreviewFamilyDueFrame": producer.get(
                "studentPersistedPreviewFamilyDueFrame"
            ),
            "studentPreviewSeedFamilyDueFrame": producer.get(
                "studentPreviewSeedFamilyDueFrame"
            ),
            "studentCurrentTeacherPreviewProbeAvailable": producer.get(
                "studentCurrentTeacherPreviewProbeAvailable"
            ),
            "studentCurrentTeacherPreviewProbeSourceDueFrame": producer.get(
                "studentCurrentTeacherPreviewProbeSourceDueFrame"
            ),
            "studentCurrentTeacherPreviewProbeStartDueFrame": producer.get(
                "studentCurrentTeacherPreviewProbeStartDueFrame"
            ),
            "studentCurrentTeacherPreviewProbeVisibleBodyPayload": producer.get(
                "studentCurrentTeacherPreviewProbeVisibleBodyPayload"
            ),
            "studentCurrentLaneMaterializationAllowed": producer.get(
                "studentCurrentLaneMaterializationAllowed"
            ),
            "studentTeacherTailOverlapTimingCandidateAvailable": producer.get(
                "studentTeacherTailOverlapTimingCandidateAvailable"
            ),
            "studentTeacherTailOverlapTimingCandidateStartFrame": producer.get(
                "studentTeacherTailOverlapTimingCandidateStartFrame"
            ),
            "studentTeacherTailOverlapTimingAnchorArmed": producer.get(
                "studentTeacherTailOverlapTimingAnchorArmed"
            ),
            "studentTeacherTailOverlapTimingAnchorStartFrame": producer.get(
                "studentTeacherTailOverlapTimingAnchorStartFrame"
            ),
            "studentTeacherTailOverlapTimingAnchorPhaseKey": producer.get(
                "studentTeacherTailOverlapTimingAnchorPhaseKey"
            ),
            "studentTeacherTailTargetWindowObserved": producer.get(
                "studentTeacherTailTargetWindowObserved"
            ),
            "studentTeacherTailTargetWindowFirstQueryFrame": producer.get(
                "studentTeacherTailTargetWindowFirstQueryFrame"
            ),
            "studentLiveSequenceSelectedSourceKind": producer.get(
                "studentLiveSequenceSelectedSourceKind"
            ),
            "studentLiveApplyBlockOrdinal": producer.get(
                "studentLiveApplyBlockOrdinal"
            ),
            "studentLiveApplyAuthorityAvailable": producer.get(
                "studentLiveApplyAuthorityAvailable"
            ),
            "studentLiveApplyMaterializationAllowed": producer.get(
                "studentLiveApplyMaterializationAllowed"
            ),
            "studentCadenceGateArmed": producer.get("studentCadenceGateArmed"),
            "studentCadenceGateStartFrame": producer.get("studentCadenceGateStartFrame"),
            "studentCadenceGatePhaseKey": producer.get("studentCadenceGatePhaseKey"),
            "active": producer.get("active"),
            "activeStreamId": producer.get("activeStreamId"),
            "rowMode": producer.get("rowMode"),
            "streamA": producer.get("streamA"),
            "streamB": producer.get("streamB"),
        },
        "live": {
            "teacher": cursor.get("teacher"),
            "student": cursor.get("student"),
            "studentBodyPreview": cursor.get("studentBodyPreview"),
        },
        "runtimeSlots": {
            "valid": runtime_slots.get("valid"),
            "queryFrame": runtime_slots.get("queryFrame"),
            "mode": runtime_slots.get("mode"),
            "onMotion": runtime_slots.get("onMotion"),
            "paMotion": runtime_slots.get("paMotion"),
        },
        "nativeRailConsumer": native_rail_consumer,
        "overlayRender": overlay_render,
    }


def _actor_signature(actor: dict[str, Any] | None) -> tuple[Any, ...]:
    actor = actor or {}
    return (
        actor.get("available"),
        actor.get("seeded"),
        actor.get("seedEnable"),
        actor.get("startDueFrame"),
        actor.get("seedCursor"),
        actor.get("currentCursor"),
        actor.get("currentEnable"),
        actor.get("portraitVisible"),
        actor.get("effectiveCursor"),
        actor.get("rollFrame"),
        actor.get("portraitCenterX"),
        tuple(actor.get("primaryStream18") or ()),
        tuple(actor.get("secondaryStream18") or ()),
    )


def _candidate_signature(candidate: dict[str, Any] | None) -> tuple[Any, ...]:
    candidate = candidate or {}
    return (
        candidate.get("available"),
        candidate.get("recordIndex"),
        candidate.get("secondaryRecordIndex"),
        candidate.get("eventIndex"),
        candidate.get("familyStartIndex"),
        candidate.get("dueFrame"),
        candidate.get("familyStartDueFrame"),
        candidate.get("rowMode"),
        tuple(candidate.get("streamA") or ()),
        tuple(candidate.get("streamB") or ()),
    )


def _preview_shadow_signature(shadow: dict[str, Any] | None) -> tuple[Any, ...]:
    shadow = shadow or {}
    source_kind = shadow.get("sourceKind") or {}
    return (
        shadow.get("available"),
        source_kind.get("name"),
        source_kind.get("value"),
        shadow.get("sourceEventPsxAddr"),
        shadow.get("familyStartEventPsxAddr"),
        shadow.get("sourceEventIndex"),
        shadow.get("familyStartIndex"),
        shadow.get("sourceDueFrame"),
        shadow.get("startDueFrame"),
        shadow.get("recordIndex"),
        shadow.get("secondaryRecordIndex"),
        shadow.get("rowMode"),
        shadow.get("seedCursor"),
        shadow.get("seedEnable"),
        shadow.get("firstVisibleCursor"),
        shadow.get("currentCursor"),
        shadow.get("currentEnable"),
        shadow.get("portraitVisible"),
        shadow.get("effectiveCursor"),
        shadow.get("rollFrame"),
        tuple(shadow.get("streamA") or ()),
        tuple(shadow.get("streamB") or ()),
    )


def _runtime_slot_signature(slot: dict[str, Any] | None) -> tuple[Any, ...]:
    slot = slot or {}
    return (
        slot.get("valid"),
        slot.get("pairIndex"),
        slot.get("startFrame"),
        slot.get("sourceEventPsxAddr"),
        slot.get("datHandle"),
        slot.get("vdfHandle"),
    )


def _change_tags(prev_record: dict[str, Any] | None, record: dict[str, Any]) -> list[str]:
    if prev_record is None:
        return ["capture_start"]

    tags: list[str] = []
    prev_parsed = (prev_record.get("rail_query") or {}).get("parsed") or {}
    curr_parsed = (record.get("rail_query") or {}).get("parsed") or {}

    if prev_parsed.get("parse_ok") != curr_parsed.get("parse_ok"):
        tags.append("parse_state_change")

    prev_rail = prev_parsed.get("rail") or {}
    curr_rail = curr_parsed.get("rail") or {}
    prev_cursor = prev_parsed.get("cursor") or {}
    curr_cursor = curr_parsed.get("cursor") or {}
    prev_producer = prev_cursor.get("producer") or {}
    curr_producer = curr_cursor.get("producer") or {}
    prev_gate = prev_parsed.get("displayGate") or {}
    curr_gate = curr_parsed.get("displayGate") or {}
    prev_runtime = prev_parsed.get("runtimeSlots") or {}
    curr_runtime = curr_parsed.get("runtimeSlots") or {}

    if prev_rail.get("rowMode") != curr_rail.get("rowMode"):
        tags.append("row_mode_change")
    if tuple(prev_rail.get("bodyStreamA") or ()) != tuple(curr_rail.get("bodyStreamA") or ()):
        tags.append("body_stream_change")
    if prev_gate.get("lessonSelector") != curr_gate.get("lessonSelector"):
        tags.append("lesson_selector_change")
    if _actor_signature(prev_cursor.get("teacher")) != _actor_signature(curr_cursor.get("teacher")):
        tags.append("teacher_live_change")
    if _actor_signature(prev_cursor.get("student")) != _actor_signature(curr_cursor.get("student")):
        tags.append("student_live_change")

    prev_source = prev_producer.get("teacherSource") or {}
    curr_source = curr_producer.get("teacherSource") or {}
    if (prev_source.get("name"), prev_source.get("value")) != (curr_source.get("name"), curr_source.get("value")):
        tags.append("teacher_source_change")
    if _candidate_signature(prev_producer.get("teacherCandidate")) != _candidate_signature(
        curr_producer.get("teacherCandidate")
    ):
        tags.append("teacher_candidate_change")
    if _candidate_signature(prev_producer.get("latestTeacherFamily")) != _candidate_signature(
        curr_producer.get("latestTeacherFamily")
    ):
        tags.append("latest_teacher_family_change")
    if _candidate_signature(prev_producer.get("nextTeacherPreviewFamily")) != _candidate_signature(
        curr_producer.get("nextTeacherPreviewFamily")
    ):
        tags.append("next_teacher_preview_family_change")
    if _preview_shadow_signature(prev_producer.get("teacherPreviewShadow")) != _preview_shadow_signature(
        curr_producer.get("teacherPreviewShadow")
    ):
        tags.append("teacher_preview_shadow_change")
    if _runtime_slot_signature(prev_runtime.get("onMotion")) != _runtime_slot_signature(curr_runtime.get("onMotion")):
        tags.append("runtime_on_motion_change")
    if _runtime_slot_signature(prev_runtime.get("paMotion")) != _runtime_slot_signature(curr_runtime.get("paMotion")):
        tags.append("runtime_pa_motion_change")
    return tags


def _actor_live_active(actor: dict[str, Any] | None) -> bool:
    actor = actor or {}
    return bool(actor.get("portraitVisible"))


def _make_marker(name: str, record: dict[str, Any], reason: str) -> dict[str, Any]:
    parsed = (record.get("rail_query") or {}).get("parsed") or {}
    rail = parsed.get("rail") or {}
    focus = record.get("focus") or {}
    return {
        "name": name,
        "reason": reason,
        "sample_index": record.get("sample_index"),
        "elapsed_ms": record.get("elapsed_ms"),
        "captured_at": record.get("captured_at"),
        "stageFrame": (record.get("stage_status") or {}).get("stageFrame"),
        "queryFrame": rail.get("queryFrame"),
        "rowMode": rail.get("rowMode"),
        "lessonSelector": ((focus.get("displayGate") or {}).get("lessonSelector")),
        "teacherSource": (((focus.get("producer") or {}).get("teacherSource") or {}).get("name")),
        "teacherLive": ((focus.get("live") or {}).get("teacher")),
        "studentLive": ((focus.get("live") or {}).get("student")),
        "teacherPreviewShadow": ((focus.get("producer") or {}).get("teacherPreviewShadow")),
    }


def _update_markers(
    summary: dict[str, Any],
    prev_record: dict[str, Any] | None,
    record: dict[str, Any],
) -> list[str]:
    markers_hit: list[str] = []
    curr_focus = record.get("focus") or {}
    curr_live = curr_focus.get("live") or {}
    curr_producer = curr_focus.get("producer") or {}
    curr_teacher = curr_live.get("teacher") or {}
    curr_student = curr_live.get("student") or {}
    teacher_active = _actor_live_active(curr_live.get("teacher"))
    student_active = _actor_live_active(curr_live.get("student"))
    teacher_source = (curr_producer.get("teacherSource") or {}).get("name")
    teacher_cursor = curr_teacher.get("effectiveCursor")
    student_cursor = curr_student.get("effectiveCursor")
    student_preview_seed_source_kind = curr_producer.get(
        "studentPreviewSeedSourceKind"
    )
    student_live_timing_source_kind = curr_producer.get(
        "studentLiveTimingSelectedSourceKind"
    )
    student_current_lane_materialization_allowed = bool(
        curr_producer.get("studentCurrentLaneMaterializationAllowed")
    )
    later_cycle_student_bootstrap_active = (
        teacher_source == "preview"
        and student_preview_seed_source_kind == 4
        and student_live_timing_source_kind == 5
        and student_current_lane_materialization_allowed
    )

    prev_teacher_active = False
    prev_student_active = False
    prev_teacher_cursor = None
    prev_student_cursor = None
    prev_teacher_source = None
    prev_later_cycle_student_bootstrap_active = False
    if prev_record is not None:
        prev_focus = prev_record.get("focus") or {}
        prev_live = prev_focus.get("live") or {}
        prev_producer = prev_focus.get("producer") or {}
        prev_teacher_active = _actor_live_active(prev_live.get("teacher"))
        prev_student_active = _actor_live_active(prev_live.get("student"))
        prev_teacher_cursor = ((prev_live.get("teacher") or {}).get("effectiveCursor"))
        prev_student_cursor = ((prev_live.get("student") or {}).get("effectiveCursor"))
        prev_teacher_source = (prev_producer.get("teacherSource") or {}).get("name")
        prev_later_cycle_student_bootstrap_active = (
            prev_teacher_source == "preview"
            and prev_producer.get("studentPreviewSeedSourceKind") == 4
            and prev_producer.get("studentLiveTimingSelectedSourceKind") == 5
            and bool(prev_producer.get("studentCurrentLaneMaterializationAllowed"))
        )

    later_cycle_bootstrap_entry_segment = (
        later_cycle_student_bootstrap_active
        and teacher_cursor == 9
        and student_cursor == 1
    )
    later_cycle_bootstrap_student_run_segment = (
        later_cycle_student_bootstrap_active
        and teacher_cursor == -1
        and student_cursor is not None
        and student_cursor >= 2
    )
    later_cycle_bootstrap_teacher1_student9_segment = (
        later_cycle_student_bootstrap_active
        and teacher_cursor == 1
        and student_cursor == 9
    )
    later_cycle_bootstrap_release_segment = (
        teacher_source == "preview"
        and teacher_cursor is not None
        and teacher_cursor >= 2
        and student_cursor == -1
        and prev_later_cycle_student_bootstrap_active
    )

    if prev_record is None:
        if teacher_active and summary["windows"]["first_teacher_start"] is None:
            summary["windows"]["first_teacher_start"] = _make_marker(
                "first_teacher_start",
                record,
                "initial_active",
            )
            markers_hit.append("first_teacher_start")
        if (
            student_active
            and summary["windows"]["first_teacher_start"] is not None
            and summary["windows"]["first_student_start"] is None
        ):
            summary["windows"]["first_student_start"] = _make_marker(
                "first_student_start",
                record,
                "initial_active",
            )
            markers_hit.append("first_student_start")
        return markers_hit

    if teacher_active and not prev_teacher_active:
        if summary["windows"]["first_teacher_start"] is None:
            summary["windows"]["first_teacher_start"] = _make_marker(
                "first_teacher_start",
                record,
                "teacher_live_rising_edge",
            )
            markers_hit.append("first_teacher_start")
        elif (
            summary["windows"]["first_teacher_end"] is not None
            and summary["windows"]["second_teacher_start"] is None
            and teacher_source == "preview"
        ):
            summary["windows"]["second_teacher_start"] = _make_marker(
                "second_teacher_start",
                record,
                "teacher_live_rising_edge",
            )
            markers_hit.append("second_teacher_start")

    if (
        student_active
        and not prev_student_active
        and summary["windows"]["first_teacher_start"] is not None
        and summary["windows"]["first_student_start"] is None
    ):
        summary["windows"]["first_student_start"] = _make_marker(
            "first_student_start",
            record,
            "student_live_rising_edge",
        )
        markers_hit.append("first_student_start")

    if (
        (not teacher_active)
        and prev_teacher_active
        and summary["windows"]["first_teacher_start"] is not None
        and summary["windows"]["first_teacher_end"] is None
    ):
        summary["windows"]["first_teacher_end"] = _make_marker(
            "first_teacher_end",
            record,
            "teacher_live_falling_edge",
        )
        markers_hit.append("first_teacher_end")

    if (
        summary["windows"].get("first_overlap_teacher8_student_hidden") is None
        and summary["windows"].get("first_teacher_end") is not None
        and summary["windows"].get("second_teacher_start") is None
        and (
            (teacher_cursor == 8 and student_cursor == -1)
            or (
                teacher_cursor == 9
                and student_cursor == 1
                and prev_teacher_cursor is not None
                and prev_teacher_cursor >= 7
                and (prev_student_cursor is None or prev_student_cursor == -1)
            )
            or (
                teacher_cursor == -1
                and student_cursor is not None
                and student_cursor >= 1
                and prev_teacher_cursor in (8, 9)
                and (prev_student_cursor is None or prev_student_cursor == -1)
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher8_student_hidden"
        if not (teacher_cursor == 8 and student_cursor == -1) and prev_record is not None:
            marker_record = prev_record
            marker_reason = (
                "teacher8_student_hidden_inferred_from_teacher9_student1"
                if teacher_cursor == 9 and student_cursor == 1
                else "teacher8_student_hidden_inferred_from_hidden_student"
            )
        summary["windows"]["first_overlap_teacher8_student_hidden"] = _make_marker(
            "first_overlap_teacher8_student_hidden",
            marker_record,
            marker_reason,
        )
        markers_hit.append("first_overlap_teacher8_student_hidden")

    if (
        summary["windows"].get("first_overlap_teacher9_student1") is None
        and summary["windows"].get("first_overlap_teacher8_student_hidden") is not None
        and (
            (teacher_cursor == 9 and student_cursor == 1)
            or (
                teacher_cursor == -1
                and student_cursor is not None
                and student_cursor >= 1
                and prev_teacher_cursor in (8, 9)
                and (prev_student_cursor is None or prev_student_cursor == -1)
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher9_student1"
        if not (teacher_cursor == 9 and student_cursor == 1) and prev_record is not None:
            marker_record = prev_record
            marker_reason = "teacher9_student1_inferred_from_hidden_student"
        summary["windows"]["first_overlap_teacher9_student1"] = _make_marker(
            "first_overlap_teacher9_student1",
            marker_record,
            marker_reason,
        )
        markers_hit.append("first_overlap_teacher9_student1")

    if (
        summary["windows"].get("first_overlap_teacher_hidden_student2") is None
        and summary["windows"].get("first_overlap_teacher9_student1") is not None
        and teacher_cursor == -1
        and student_cursor is not None
        and student_cursor >= 2
    ):
        summary["windows"]["first_overlap_teacher_hidden_student2"] = _make_marker(
            "first_overlap_teacher_hidden_student2",
            record,
            "teacher_hidden_student_ge2",
        )
        markers_hit.append("first_overlap_teacher_hidden_student2")

    if (
        summary["windows"].get("second_overlap_teacher_hidden_student8") is None
        and summary["windows"].get("second_teacher_start") is not None
        and not later_cycle_student_bootstrap_active
        and not later_cycle_bootstrap_release_segment
        and (
            (teacher_cursor == -1 and student_cursor == 8)
            or (
                teacher_cursor == 1
                and student_cursor == 9
            )
            or (
                teacher_source == "preview"
                and teacher_cursor == 1
                and student_cursor == -1
                and prev_teacher_cursor == -1
                and prev_student_cursor is not None
                and prev_student_cursor >= 7
            )
            or (
                teacher_source == "preview"
                and teacher_cursor is not None
                and teacher_cursor >= 2
                and student_cursor == -1
                and prev_teacher_cursor == -1
                and prev_student_cursor is not None
                and prev_student_cursor >= 7
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher_hidden_student8"
        if not (teacher_cursor == -1 and student_cursor == 8) and prev_record is not None:
            if teacher_cursor == 1 and student_cursor == 9:
                marker_record = record
                marker_reason = "teacher_hidden_student8_inferred_from_teacher1_student9"
            elif (
                teacher_source == "preview"
                and teacher_cursor == 1
                and student_cursor == -1
            ):
                marker_record = prev_record
                marker_reason = (
                    "teacher_hidden_student8_inferred_from_teacher1_hidden"
                )
            else:
                marker_record = prev_record
                marker_reason = (
                    "teacher_hidden_student8_inferred_from_teacher_ge2_hidden"
                    if teacher_source == "preview"
                    and teacher_cursor is not None
                    and teacher_cursor >= 2
                    and student_cursor == -1
                    else "teacher_hidden_student8_inferred_from_next_overlap"
                )
        summary["windows"]["second_overlap_teacher_hidden_student8"] = _make_marker(
            "second_overlap_teacher_hidden_student8",
            marker_record,
            marker_reason,
        )
        markers_hit.append("second_overlap_teacher_hidden_student8")

    if (
        summary["windows"].get("second_overlap_teacher1_student9") is None
        and summary["windows"].get("second_overlap_teacher_hidden_student8") is not None
        and not later_cycle_student_bootstrap_active
        and not later_cycle_bootstrap_release_segment
        and (
            (teacher_cursor == 1 and student_cursor == 9)
            or (
                teacher_source == "preview"
                and teacher_cursor == 1
                and student_cursor == -1
                and prev_teacher_cursor == -1
                and prev_student_cursor is not None
                and prev_student_cursor >= 7
            )
            or (
                teacher_source == "preview"
                and teacher_cursor is not None
                and teacher_cursor >= 2
                and student_cursor == -1
                and prev_teacher_cursor == -1
                and prev_student_cursor is not None
                and prev_student_cursor >= 7
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher1_student9"
        if not (teacher_cursor == 1 and student_cursor == 9) and prev_record is not None:
            marker_record = record
            marker_reason = (
                "teacher1_student9_inferred_from_teacher1_hidden"
                if teacher_source == "preview"
                and teacher_cursor == 1
                and student_cursor == -1
                else "teacher1_student9_inferred_from_teacher_ge2_hidden"
            )
        summary["windows"]["second_overlap_teacher1_student9"] = _make_marker(
            "second_overlap_teacher1_student9",
            marker_record,
            marker_reason,
        )
        markers_hit.append("second_overlap_teacher1_student9")

    if (
        summary["windows"].get("second_overlap_teacher2_student_hidden") is None
        and summary["windows"].get("second_overlap_teacher1_student9") is not None
        and not later_cycle_student_bootstrap_active
        and not later_cycle_bootstrap_release_segment
        and teacher_cursor is not None
        and teacher_cursor >= 2
        and student_cursor == -1
    ):
        summary["windows"]["second_overlap_teacher2_student_hidden"] = _make_marker(
            "second_overlap_teacher2_student_hidden",
            record,
            "teacher_ge2_student_hidden",
        )
        markers_hit.append("second_overlap_teacher2_student_hidden")

    second_teacher_start_marker = summary["windows"].get("second_teacher_start") or {}
    same_sample_as_second_teacher_start = (
        second_teacher_start_marker.get("sample_index") is not None
        and second_teacher_start_marker.get("sample_index") == record.get("sample_index")
    )
    old_current_lane_release_wait_tail = (
        summary["windows"].get("first_overlap_teacher_hidden_student2") is not None
        and summary["windows"].get("second_teacher_start") is None
        and teacher_source == "current-lane"
        and teacher_cursor == -1
        and student_cursor is not None
        and student_cursor >= 7
        and bool(curr_producer.get("teacherFormalCurrentLaneOwnershipReady"))
        and bool(curr_producer.get("teacherFormalCurrentLaneWaitingForStudentTailTargetWindow"))
        and not bool(curr_producer.get("teacherFormalShouldPreferLiveSequenceAuthority"))
    )
    same_frame_preview_takeover_release_wait_tail = (
        summary["windows"].get("first_overlap_teacher_hidden_student2") is not None
        and same_sample_as_second_teacher_start
        and teacher_source == "preview"
        and teacher_cursor is not None
        and teacher_cursor >= 1
        and student_cursor is not None
        and student_cursor >= 7
        and bool(curr_producer.get("teacherFormalCurrentLaneWaitingForStudentTailTargetWindow"))
    )
    if (
        summary["windows"].get("current_lane_release_wait_tail") is None
        and (
            old_current_lane_release_wait_tail
            or same_frame_preview_takeover_release_wait_tail
            or (
                summary["windows"].get("first_overlap_teacher_hidden_student2") is not None
                and teacher_source == "preview"
                and teacher_cursor is not None
                and teacher_cursor >= 2
                and student_cursor == -1
                and prev_teacher_cursor == -1
                and prev_student_cursor is not None
                and prev_student_cursor >= 7
            )
        )
    ):
        summary["windows"]["current_lane_release_wait_tail"] = _make_marker(
            "current_lane_release_wait_tail",
            record,
            "current_lane_waiting_for_student_tail_target_window",
        )
        markers_hit.append("current_lane_release_wait_tail")

    if (
        summary["windows"].get("preview_hold_after_takeover") is None
        and summary["windows"].get("second_teacher_start") is not None
        and teacher_source == "preview"
        and teacher_cursor is not None
        and teacher_cursor >= 1
        and bool(curr_producer.get("teacherFormalShouldHoldPreviewOwnership"))
    ):
        summary["windows"]["preview_hold_after_takeover"] = _make_marker(
            "preview_hold_after_takeover",
            record,
            "preview_hold_after_takeover",
        )
        markers_hit.append("preview_hold_after_takeover")

    if (
        summary["windows"].get("later_cycle_teacher8_student_hidden") is None
        and summary["windows"].get("preview_hold_after_takeover") is not None
        and (
            later_cycle_bootstrap_entry_segment
            or later_cycle_bootstrap_student_run_segment
        )
    ):
        marker_reason = "later_cycle_bootstrap_entry"
        marker_record = record
        if later_cycle_bootstrap_student_run_segment and prev_record is not None:
            marker_reason = "later_cycle_bootstrap_entry_inferred_from_hidden_student"
        summary["windows"]["later_cycle_teacher8_student_hidden"] = _make_marker(
            "later_cycle_teacher8_student_hidden",
            marker_record,
            marker_reason,
        )
        markers_hit.append("later_cycle_teacher8_student_hidden")

    if (
        summary["windows"].get("later_cycle_teacher9_student1") is None
        and summary["windows"].get("later_cycle_teacher8_student_hidden") is not None
        and (
            later_cycle_bootstrap_student_run_segment
            or later_cycle_bootstrap_teacher1_student9_segment
        )
    ):
        marker_record = record
        marker_reason = "later_cycle_hidden_student_ge2"
        if later_cycle_bootstrap_teacher1_student9_segment and prev_record is not None:
            marker_record = prev_record
            marker_reason = "later_cycle_hidden_student_ge2_inferred_from_teacher1_student9"
        summary["windows"]["later_cycle_teacher9_student1"] = _make_marker(
            "later_cycle_teacher9_student1",
            marker_record,
            marker_reason,
        )
        markers_hit.append("later_cycle_teacher9_student1")

    if (
        summary["windows"].get("later_cycle_teacher_hidden_student2") is None
        and summary["windows"].get("later_cycle_teacher9_student1") is not None
        and (
            later_cycle_bootstrap_teacher1_student9_segment
            or later_cycle_bootstrap_release_segment
        )
    ):
        marker_reason = "later_cycle_teacher1_student9"
        marker_record = record
        if later_cycle_bootstrap_release_segment and prev_record is not None:
            marker_record = prev_record
            marker_reason = "later_cycle_teacher1_student9_inferred_from_release"
        summary["windows"]["later_cycle_teacher_hidden_student2"] = _make_marker(
            "later_cycle_teacher_hidden_student2",
            marker_record,
            marker_reason,
        )
        markers_hit.append("later_cycle_teacher_hidden_student2")

    if (
        summary["windows"].get("late_repeat_teacher_hidden_student8") is None
        and summary["windows"].get("later_cycle_teacher_hidden_student2") is not None
        and teacher_cursor == -1
        and student_cursor == 8
    ):
        summary["windows"]["late_repeat_teacher_hidden_student8"] = _make_marker(
            "late_repeat_teacher_hidden_student8",
            record,
            "teacher_hidden_student8",
        )
        markers_hit.append("late_repeat_teacher_hidden_student8")

    if (
        summary["windows"].get("late_repeat_teacher1_student9") is None
        and summary["windows"].get("late_repeat_teacher_hidden_student8") is not None
        and (
            (teacher_cursor == 1 and student_cursor == 9)
            or (
                isinstance(teacher_cursor, int)
                and teacher_cursor >= 1
                and student_cursor == -1
                and prev_teacher_cursor == -1
                and prev_student_cursor == 8
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher1_student9"
        if not (teacher_cursor == 1 and student_cursor == 9) and prev_record is not None:
            marker_record = prev_record
            marker_reason = "teacher1_student9_inferred_from_teacher_ge2_student_hidden"
        summary["windows"]["late_repeat_teacher1_student9"] = _make_marker(
            "late_repeat_teacher1_student9",
            marker_record,
            marker_reason,
        )
        markers_hit.append("late_repeat_teacher1_student9")

    if (
        summary["windows"].get("late_repeat_teacher2_student_hidden") is None
        and summary["windows"].get("late_repeat_teacher1_student9") is not None
        and isinstance(teacher_cursor, int)
        and teacher_cursor >= 2
        and student_cursor == -1
    ):
        summary["windows"]["late_repeat_teacher2_student_hidden"] = _make_marker(
            "late_repeat_teacher2_student_hidden",
            record,
            "teacher_ge2_student_hidden",
        )
        markers_hit.append("late_repeat_teacher2_student_hidden")

    if (
        summary["windows"].get("late_repeat_preview_teacher9_student1") is None
        and summary["windows"].get("late_repeat_teacher2_student_hidden") is not None
        and teacher_cursor == 9
        and student_cursor == 1
    ):
        summary["windows"]["late_repeat_preview_teacher9_student1"] = _make_marker(
            "late_repeat_preview_teacher9_student1",
            record,
            "teacher9_student1",
        )
        markers_hit.append("late_repeat_preview_teacher9_student1")

    if (
        summary["windows"].get("late_repeat_preview_teacher_hidden_student2") is None
        and summary["windows"].get("late_repeat_preview_teacher9_student1") is not None
        and (
            (
                teacher_cursor == -1
                and isinstance(student_cursor, int)
                and student_cursor >= 2
            )
            or (
                isinstance(teacher_cursor, int)
                and teacher_cursor >= 2
                and isinstance(student_cursor, int)
                and student_cursor >= 2
            )
        )
    ):
        marker_reason = "teacher_hidden_student_ge2"
        if not (teacher_cursor == -1 and isinstance(student_cursor, int) and student_cursor >= 2):
            marker_reason = "teacher_hidden_student_ge2_inferred_from_teacher_ge2_student_ge2"
        summary["windows"]["late_repeat_preview_teacher_hidden_student2"] = _make_marker(
            "late_repeat_preview_teacher_hidden_student2",
            record,
            marker_reason,
        )
        markers_hit.append("late_repeat_preview_teacher_hidden_student2")

    if (
        summary["windows"].get("late_repeat_preview_teacher_hidden_student8") is None
        and summary["windows"].get("late_repeat_preview_teacher_hidden_student2") is not None
        and (
            (teacher_cursor == -1 and student_cursor == 8)
            or (
                prev_teacher_cursor == 8
                and prev_student_cursor == 8
                and isinstance(teacher_cursor, int)
                and teacher_cursor >= 1
                and student_cursor == -1
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher_hidden_student8"
        if not (teacher_cursor == -1 and student_cursor == 8) and prev_record is not None:
            marker_record = prev_record
            marker_reason = "teacher_hidden_student8_inferred_from_teacher_ge1_student_hidden"
        summary["windows"]["late_repeat_preview_teacher_hidden_student8"] = _make_marker(
            "late_repeat_preview_teacher_hidden_student8",
            marker_record,
            marker_reason,
        )
        markers_hit.append("late_repeat_preview_teacher_hidden_student8")

    if (
        summary["windows"].get("late_repeat_second_teacher1_student9") is None
        and summary["windows"].get("late_repeat_preview_teacher_hidden_student8") is not None
        and (
            (teacher_cursor == 1 and student_cursor == 9)
            or (
                isinstance(teacher_cursor, int)
                and teacher_cursor >= 1
                and student_cursor == -1
            )
        )
    ):
        marker_record = record
        marker_reason = "teacher1_student9"
        if not (teacher_cursor == 1 and student_cursor == 9):
            marker_reason = "teacher1_student9_inferred_from_teacher_ge2_student_hidden"
        summary["windows"]["late_repeat_second_teacher1_student9"] = _make_marker(
            "late_repeat_second_teacher1_student9",
            marker_record,
            marker_reason,
        )
        markers_hit.append("late_repeat_second_teacher1_student9")

    if (
        summary["windows"].get("late_repeat_second_teacher2_student_hidden") is None
        and summary["windows"].get("late_repeat_second_teacher1_student9") is not None
        and isinstance(teacher_cursor, int)
        and teacher_cursor >= 2
        and student_cursor == -1
    ):
        summary["windows"]["late_repeat_second_teacher2_student_hidden"] = _make_marker(
            "late_repeat_second_teacher2_student_hidden",
            record,
            "teacher_ge2_student_hidden",
        )
        markers_hit.append("late_repeat_second_teacher2_student_hidden")

    return markers_hit


def _extract_window(records: list[dict[str, Any]], marker: dict[str, Any] | None, pre: int, post: int) -> dict[str, Any]:
    if marker is None:
        return {
            "detected": False,
            "marker": None,
            "sample_start": None,
            "sample_end": None,
            "samples": [],
        }

    center = int(marker["sample_index"])
    sample_start = max(0, center - pre)
    sample_end = min(len(records) - 1, center + post)
    return {
        "detected": True,
        "marker": marker,
        "sample_start": sample_start,
        "sample_end": sample_end,
        "samples": records[sample_start: sample_end + 1],
    }


def _boot_stage1(tester: DispatcherTester, boot_mode: str) -> bool:
    if boot_mode == "attach":
        deadline = time.monotonic() + 30.0
        while time.monotonic() < deadline:
            status = _extract_status_subset(tester.client.json_status())
            if status.get("scene") == 1 and status.get("stageRunning") == 1:
                return True
            time.sleep(0.1)
        return False
    if boot_mode == "direct":
        return tester._enter_stage1_via_direct_boot()
    if not tester._restart_game_and_wait_debug_ready(15.0):
        return False
    if not tester.ensure_scn0_full_menu(timeout=60.0):
        return False
    return tester._enter_stage1_from_current_scene0_stage_select()


def _summary_text(output_dir: Path, session_payload: dict[str, Any]) -> str:
    summary = session_payload["summary"]
    files = session_payload["files"]
    lines = [
        "Win-only Stage1 teacher rail capture",
        f"captured_at={session_payload['captured_at']}",
        f"label={session_payload['label']}",
        f"boot_mode={session_payload['boot_mode']}",
        f"stop_on_second_tail={session_payload['stop_on_second_tail']}",
        f"stop_reason={session_payload['stop_reason']}",
        f"samples={summary['samples_captured']}",
        f"runtime_available_samples={summary['runtime_available_samples']}",
        f"parse_ok_samples={summary['parse_ok_samples']}",
        f"status_refreshes={summary['status_refreshes']}",
        f"avg_rail_query_ms={summary['avg_rail_query_ms']}",
        f"max_rail_query_ms={summary['max_rail_query_ms']}",
        f"avg_loop_ms={summary['avg_loop_ms']}",
        f"row_modes_seen={summary['row_modes_seen']}",
        f"lesson_selectors_seen={summary['lesson_selectors_seen']}",
        f"teacher_sources_seen={summary['teacher_sources_seen']}",
        f"first_teacher_start={summary['windows']['first_teacher_start'] is not None}",
        f"first_student_start={summary['windows']['first_student_start'] is not None}",
        f"first_teacher_end={summary['windows']['first_teacher_end'] is not None}",
        f"second_teacher_start={summary['windows']['second_teacher_start'] is not None}",
        f"first_overlap_teacher8_student_hidden={summary['windows']['first_overlap_teacher8_student_hidden'] is not None}",
        f"first_overlap_teacher9_student1={summary['windows']['first_overlap_teacher9_student1'] is not None}",
        f"first_overlap_teacher_hidden_student2={summary['windows']['first_overlap_teacher_hidden_student2'] is not None}",
        f"second_overlap_teacher_hidden_student8={summary['windows']['second_overlap_teacher_hidden_student8'] is not None}",
        f"second_overlap_teacher1_student9={summary['windows']['second_overlap_teacher1_student9'] is not None}",
        f"second_overlap_teacher2_student_hidden={summary['windows']['second_overlap_teacher2_student_hidden'] is not None}",
        f"current_lane_release_wait_tail={summary['windows']['current_lane_release_wait_tail'] is not None}",
        f"preview_hold_after_takeover={summary['windows']['preview_hold_after_takeover'] is not None}",
        f"later_cycle_teacher8_student_hidden={summary['windows']['later_cycle_teacher8_student_hidden'] is not None}",
        f"later_cycle_teacher9_student1={summary['windows']['later_cycle_teacher9_student1'] is not None}",
        f"later_cycle_teacher_hidden_student2={summary['windows']['later_cycle_teacher_hidden_student2'] is not None}",
        f"late_repeat_teacher_hidden_student8={summary['windows']['late_repeat_teacher_hidden_student8'] is not None}",
        f"late_repeat_teacher1_student9={summary['windows']['late_repeat_teacher1_student9'] is not None}",
        f"late_repeat_teacher2_student_hidden={summary['windows']['late_repeat_teacher2_student_hidden'] is not None}",
        f"late_repeat_preview_teacher9_student1={summary['windows']['late_repeat_preview_teacher9_student1'] is not None}",
        f"late_repeat_preview_teacher_hidden_student2={summary['windows']['late_repeat_preview_teacher_hidden_student2'] is not None}",
        f"late_repeat_preview_teacher_hidden_student8={summary['windows']['late_repeat_preview_teacher_hidden_student8'] is not None}",
        f"late_repeat_second_teacher1_student9={summary['windows']['late_repeat_second_teacher1_student9'] is not None}",
        f"late_repeat_second_teacher2_student_hidden={summary['windows']['late_repeat_second_teacher2_student_hidden'] is not None}",
        f"timeline={files['timeline']}",
        f"keyframes={files['keyframes']}",
        f"windows={files['windows']}",
        f"session={files['session']}",
        f"summary={files['summary']}",
        f"artifact_dir={output_dir}",
    ]
    return "\n".join(lines) + "\n"


def _stop_reason(
    args: argparse.Namespace,
    record: dict[str, Any],
    prev_record: dict[str, Any] | None,
    summary: dict[str, Any],
) -> str | None:
    if STOP_REQUESTED:
        return "signal"

    parsed = (record.get("rail_query") or {}).get("parsed") or {}
    status = record.get("stage_status") or {}
    rail_query_frame = ((parsed.get("rail") or {}).get("queryFrame"))

    stop_on_second_tail = bool(getattr(args, "stop_on_second_tail", True))
    if stop_on_second_tail and summary["windows"]["second_teacher_start"] is not None:
        target_query = (summary["windows"]["second_teacher_start"] or {}).get("queryFrame")
        if isinstance(target_query, int) and isinstance(rail_query_frame, int):
            if rail_query_frame >= target_query + int(args.tail_query_frames):
                return "second_teacher_tail_complete"

    if isinstance(rail_query_frame, int) and rail_query_frame >= int(args.max_query_frame):
        return "max_query_frame"

    stage_frame = status.get("stageFrame")
    if isinstance(stage_frame, int) and stage_frame >= int(args.max_stage_frame):
        return "max_stage_frame"

    scene = status.get("scene")
    stage_running = status.get("stageRunning")
    if summary["samples_captured"] > 0 and scene is not None and stage_running is not None:
        if scene != 1 or stage_running != 1:
            return "left_stage1"

    if args.seconds > 0 and record["elapsed_ms"] >= int(args.seconds * 1000.0):
        return "seconds_limit"

    return None


def _make_record(
    sample_index: int,
    elapsed_ms: int,
    raw: str,
    parsed: dict[str, Any],
    status: dict[str, Any],
    timing: dict[str, Any],
    status_fresh: bool,
) -> dict[str, Any]:
    return {
        "api": "openai-responses",
        "capture_kind": CAPTURE_KIND,
        "sample_index": sample_index,
        "captured_at": _dt.datetime.now().isoformat(),
        "elapsed_ms": elapsed_ms,
        "stage_status": status,
        "stage_status_fresh": status_fresh,
        "timing_ms": timing,
        "focus": _make_focus(parsed),
        "rail_query": {
            "raw": raw,
            "parsed": parsed,
        },
    }


def _finalize_summary(summary: dict[str, Any]) -> dict[str, Any]:
    samples = max(1, summary["samples_captured"])
    summary["avg_rail_query_ms"] = round(summary["rail_query_ms_total"] / samples, 2)
    summary["avg_loop_ms"] = round(summary["loop_ms_total"] / samples, 2)
    summary["row_modes_seen"] = sorted(summary["row_modes_seen"])
    summary["lesson_selectors_seen"] = sorted(summary["lesson_selectors_seen"])
    summary["teacher_sources_seen"] = sorted(summary["teacher_sources_seen"])
    del summary["rail_query_ms_total"]
    del summary["loop_ms_total"]
    return summary


def _self_test() -> int:
    raw = (
        "rail.valid=1 rail.queryFrame=456 rail.rowMode=1\n"
        "rail.bodyStreamA[18]=[-1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "currentCommittedCue.available=1 currentCommittedCue.symbolType=3\n"
        "nextLookaheadCue.available=1 nextLookaheadCue.symbolType=2 "
        "rail.nextLookaheadCueWindowActive=1 rail.nextLookaheadAcceptedContributionEnabled=0\n"
        "acceptedBoundaryCue.available=1 acceptedBoundaryCue.symbolType=4 "
        "rail.acceptedBoundarySourceCellGateActive=1 rail.acceptedBoundaryMaterialized=0 "
        "rail.acceptedHandoffPhase=CueWindow(1)\n"
        "cursor.valid=1 cursor.queryFrame=456 cursor.rowMode=1\n"
        "cursor.producer.active=1 cursor.producer.activeStreamId=1 "
        "cursor.producer.lastAppliedEventPsxAddr=0x80112233 "
        "cursor.producer.lastAppliedEventDueFrame=440 cursor.producer.activeEventAvailable=1 "
        "cursor.producer.activeEventDueFrame=456 cursor.producer.streamBaseDueFrame=432 "
        "cursor.producer.activeEventIndex=5 cursor.producer.recordIndex=7 "
        "cursor.producer.secondaryRecordIndex=8 cursor.producer.rowMode=1 "
        "cursor.producer.teacherSource=preview(2) "
        "cursor.producer.streamA[18]=[-1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "cursor.producer.streamB[18]=[-1,8,7,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "cursor.producer.teacherCandidate.available=1 cursor.producer.teacherCandidate.recordIndex=4 "
        "cursor.producer.teacherCandidate.secondaryRecordIndex=5 cursor.producer.teacherCandidate.eventIndex=10 "
        "cursor.producer.teacherCandidate.familyStartIndex=9 cursor.producer.teacherCandidate.dueFrame=470 "
        "cursor.producer.teacherCandidate.familyStartDueFrame=462 cursor.producer.teacherCandidate.rowMode=1 "
        "cursor.producer.teacherCandidate.streamA[18]=[-1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "cursor.producer.teacherCandidate.streamB[18]=[-1,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "cursor.producer.latestTeacherFamily.available=1 cursor.producer.latestTeacherFamily.recordIndex=11 "
        "cursor.producer.latestTeacherFamily.secondaryRecordIndex=12 cursor.producer.latestTeacherFamily.eventIndex=20 "
        "cursor.producer.latestTeacherFamily.familyStartIndex=18 cursor.producer.latestTeacherFamily.dueFrame=420 "
        "cursor.producer.latestTeacherFamily.familyStartDueFrame=412 cursor.producer.latestTeacherFamily.rowMode=1 "
        "cursor.producer.latestTeacherFamily.streamA[18]=[-1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "cursor.producer.latestTeacherFamily.streamB[18]=[-1,9,9,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "cursor.producer.nextTeacherPreviewFamily.available=1 cursor.producer.nextTeacherPreviewFamily.recordIndex=13 "
        "cursor.producer.nextTeacherPreviewFamily.secondaryRecordIndex=14 "
        "cursor.producer.nextTeacherPreviewFamily.eventIndex=30 "
        "cursor.producer.nextTeacherPreviewFamily.familyStartIndex=28 "
        "cursor.producer.nextTeacherPreviewFamily.dueFrame=520 "
        "cursor.producer.nextTeacherPreviewFamily.familyStartDueFrame=508 "
        "cursor.producer.nextTeacherPreviewFamily.rowMode=1 "
        "cursor.producer.nextTeacherPreviewFamily.streamA[18]=[-1,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "cursor.producer.nextTeacherPreviewFamily.streamB[18]=[-1,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "cursor.producer.teacherPreviewShadow.available=1 cursor.producer.teacherPreviewShadow.sourceKind=preview(2) "
        "cursor.producer.teacherPreviewShadow.sourceEventPsxAddr=0x80112244 "
        "cursor.producer.teacherPreviewShadow.familyStartEventPsxAddr=0x80112240 "
        "cursor.producer.teacherPreviewShadow.sourceEventIndex=30 "
        "cursor.producer.teacherPreviewShadow.familyStartIndex=28 "
        "cursor.producer.teacherPreviewShadow.sourceDueFrame=520 "
        "cursor.producer.teacherPreviewShadow.startDueFrame=508 "
        "cursor.producer.teacherPreviewShadow.recordIndex=13 "
        "cursor.producer.teacherPreviewShadow.secondaryRecordIndex=14 "
        "cursor.producer.teacherPreviewShadow.rowMode=1 cursor.producer.teacherPreviewShadow.seedCursor=1 "
        "cursor.producer.teacherPreviewShadow.seedEnable=1 cursor.producer.teacherPreviewShadow.firstVisibleCursor=1 "
        "cursor.producer.teacherPreviewShadow.currentCursor=2 cursor.producer.teacherPreviewShadow.currentEnable=1 "
        "cursor.producer.teacherPreviewShadow.portraitVisible=1 cursor.producer.teacherPreviewShadow.effectiveCursor=2 "
        "cursor.producer.teacherPreviewShadow.rollFrame=3 "
        "cursor.producer.teacherPreviewShadow.streamA[18]=[-1,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "cursor.producer.teacherPreviewShadow.streamB[18]=[-1,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "teacher.available=1 teacher.seeded=1 teacher.recordIndex=1 teacher.secondaryRecordIndex=2 "
        "teacher.seedEnable=1 teacher.startDueFrame=410 teacher.seedCursor=0 teacher.currentCursor=2 "
        "teacher.currentEnable=1 teacher.portraitVisible=1 teacher.effectiveCursor=2 teacher.rollFrame=3 "
        "teacher.portraitCenterXValid=1 teacher.portraitCenterX=76 "
        "teacher.primaryStream18=[-1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "teacher.secondaryStream18=[-1,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "student.available=1 student.seeded=1 student.recordIndex=3 student.secondaryRecordIndex=4 "
        "student.seedEnable=1 student.startDueFrame=430 student.seedCursor=0 student.currentCursor=4 "
        "student.currentEnable=1 student.portraitVisible=1 student.effectiveCursor=4 student.rollFrame=2 "
        "student.portraitCenterXValid=1 student.portraitCenterX=100 "
        "student.primaryStream18=[-1,8,7,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0] "
        "student.secondaryStream18=[-1,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\n"
        "displayGate.valid=1 displayGate.queryFrame=456 displayGate.lessonSelector=2 "
        "displayGate.lessonVisible=1 displayGate.railVisibility.narrowVisibleCandidate=1 "
        "displayGate.railVisibility.cueWindowGateActive=1 displayGate.railVisibility.sourceCellGateActive=1 "
        "displayGate.railVisibility.materializedGateActive=1\n"
        "runtimeSlots.valid=1 runtimeSlots.queryFrame=456 runtimeSlots.mode=1\n"
        "runtimeSlots.onMotion.valid=1 runtimeSlots.onMotion.pairIndex=1 runtimeSlots.onMotion.startFrame=400 "
        "runtimeSlots.onMotion.sourceEventPsxAddr=0x80112300 runtimeSlots.onMotion.datHandle=20 "
        "runtimeSlots.onMotion.vdfHandle=21\n"
        "runtimeSlots.paMotion.valid=1 runtimeSlots.paMotion.pairIndex=2 runtimeSlots.paMotion.startFrame=420 "
        "runtimeSlots.paMotion.sourceEventPsxAddr=0x80112400 runtimeSlots.paMotion.datHandle=30 "
        "runtimeSlots.paMotion.vdfHandle=31"
    )
    parsed = parse_stage1ovl_rail(raw)
    assert parsed["parse_ok"] is True
    assert parsed["cursor"]["producer"]["teacherSource"]["name"] == "preview"
    assert parsed["cursor"]["producer"]["teacherCandidate"]["eventIndex"] == 10
    assert parsed["cursor"]["producer"]["teacherPreviewShadow"]["effectiveCursor"] == 2
    assert parsed["runtimeSlots"]["onMotion"]["pairIndex"] == 1
    rec0_raw = raw.replace("teacher.currentEnable=1", "teacher.currentEnable=0").replace(
        "teacher.portraitVisible=1",
        "teacher.portraitVisible=0",
    ).replace("teacher.effectiveCursor=2", "teacher.effectiveCursor=-1")
    rec0 = _make_record(
        sample_index=0,
        elapsed_ms=0,
        raw=rec0_raw,
        parsed=parse_stage1ovl_rail(rec0_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 100},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=True,
    )
    rec1 = _make_record(
        sample_index=1,
        elapsed_ms=8,
        raw=raw,
        parsed=parsed,
        status={"scene": 1, "stageRunning": 1, "stageFrame": 108},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    summary = {
        "windows": {
            "first_teacher_start": None,
            "first_student_start": None,
            "first_teacher_end": None,
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    assert _update_markers(summary, None, rec0) == []
    assert "first_teacher_start" in _update_markers(summary, rec0, rec1)

    second_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 10},
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_second_prev_raw = raw.replace(
        "teacher.currentCursor=2",
        "teacher.currentCursor=-1",
    ).replace(
        "teacher.effectiveCursor=2",
        "teacher.effectiveCursor=-1",
    ).replace(
        "teacher.portraitVisible=1",
        "teacher.portraitVisible=0",
    ).replace(
        "student.currentCursor=4",
        "student.currentCursor=8",
    ).replace(
        "student.effectiveCursor=4",
        "student.effectiveCursor=8",
    ).replace(
        "teacher.startDueFrame=410",
        "teacher.startDueFrame=0",
    )
    rec_second_prev = _make_record(
        sample_index=11,
        elapsed_ms=16,
        raw=rec_second_prev_raw,
        parsed=parse_stage1ovl_rail(rec_second_prev_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 116},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_second_curr_raw = raw.replace(
        "teacher.startDueFrame=410",
        "teacher.startDueFrame=505",
    ).replace(
        "teacher.currentCursor=2",
        "teacher.currentCursor=1",
    ).replace(
        "teacher.effectiveCursor=2",
        "teacher.effectiveCursor=1",
    ).replace(
        "student.currentCursor=4",
        "student.currentCursor=9",
    ).replace(
        "student.effectiveCursor=4",
        "student.effectiveCursor=9",
    )
    rec_second_curr = _make_record(
        sample_index=12,
        elapsed_ms=24,
        raw=rec_second_curr_raw,
        parsed=parse_stage1ovl_rail(rec_second_curr_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 124},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    hits = _update_markers(second_summary, rec_second_prev, rec_second_curr)
    assert "second_overlap_teacher_hidden_student8" in hits
    assert "second_overlap_teacher1_student9" in hits

    second_hidden_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 12},
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": {"sample_index": 22},
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_second_curr_hidden_raw = rec_second_curr_raw.replace(
        "student.currentCursor=9",
        "student.currentCursor=-1",
    ).replace(
        "student.effectiveCursor=9",
        "student.effectiveCursor=-1",
    ).replace(
        "student.portraitVisible=1",
        "student.portraitVisible=0",
    )
    rec_second_curr_hidden = _make_record(
        sample_index=12,
        elapsed_ms=24,
        raw=rec_second_curr_hidden_raw,
        parsed=parse_stage1ovl_rail(rec_second_curr_hidden_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 124},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_second_curr_hidden["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    hidden_hits = _update_markers(
        second_hidden_summary, rec_second_prev, rec_second_curr_hidden
    )
    assert "second_overlap_teacher_hidden_student8" in hidden_hits
    assert "second_overlap_teacher1_student9" in hidden_hits

    release_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": {"sample_index": 20},
            "first_overlap_teacher9_student1": {"sample_index": 21},
            "first_overlap_teacher_hidden_student2": {"sample_index": 22},
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_release_raw = rec_second_prev_raw.replace(
        "student.currentCursor=8",
        "student.currentCursor=7",
    ).replace(
        "student.effectiveCursor=8",
        "student.effectiveCursor=7",
    )
    rec_release = _make_record(
        sample_index=13,
        elapsed_ms=28,
        raw=rec_release_raw,
        parsed=parse_stage1ovl_rail(rec_release_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 128},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_release["focus"]["producer"]["teacherSource"] = {
        "name": "current-lane",
        "value": 4,
    }
    rec_release["focus"]["producer"]["teacherFormalCurrentLaneOwnershipReady"] = True
    rec_release["focus"]["producer"][
        "teacherFormalCurrentLaneWaitingForStudentTailTargetWindow"
    ] = True
    rec_release["focus"]["producer"][
        "teacherFormalShouldPreferLiveSequenceAuthority"
    ] = False
    release_hits = _update_markers(release_summary, rec_second_prev, rec_release)
    assert "current_lane_release_wait_tail" in release_hits

    release_takeover_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 14},
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": {"sample_index": 8},
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
        }
    }
    rec_release_takeover = _make_record(
        sample_index=14,
        elapsed_ms=36,
        raw=rec_second_curr_raw,
        parsed=parse_stage1ovl_rail(rec_second_curr_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 136},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_release_takeover["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    rec_release_takeover["focus"]["producer"][
        "teacherFormalCurrentLaneWaitingForStudentTailTargetWindow"
    ] = True
    rec_release_takeover["focus"]["producer"][
        "teacherFormalShouldHoldPreviewOwnership"
    ] = True
    release_takeover_hits = _update_markers(
        release_takeover_summary, rec_second_prev, rec_release_takeover
    )
    assert "current_lane_release_wait_tail" in release_takeover_hits

    preview_hold_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 10},
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
        }
    }
    rec_preview_hold = _make_record(
        sample_index=14,
        elapsed_ms=36,
        raw=rec_second_curr_raw,
        parsed=parse_stage1ovl_rail(rec_second_curr_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 136},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_preview_hold["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    rec_preview_hold["focus"]["producer"]["teacherFormalShouldHoldPreviewOwnership"] = True
    preview_hold_hits = _update_markers(
        preview_hold_summary,
        rec_second_prev,
        rec_preview_hold,
    )
    assert "preview_hold_after_takeover" in preview_hold_hits

    later_cycle_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 10},
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": {"sample_index": 30},
            "second_overlap_teacher1_student9": {"sample_index": 31},
            "second_overlap_teacher2_student_hidden": {"sample_index": 32},
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": {"sample_index": 33},
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_later_base_raw = rec_second_prev_raw.replace(
        "student.currentCursor=8",
        "student.currentCursor=-1",
    ).replace(
        "student.effectiveCursor=8",
        "student.effectiveCursor=-1",
    ).replace(
        "student.startDueFrame=420",
        "student.startDueFrame=440",
    ).replace(
        "teacher.startDueFrame=0",
        "teacher.startDueFrame=504",
    ).replace(
        "teacher.currentCursor=-1",
        "teacher.currentCursor=8",
    ).replace(
        "teacher.effectiveCursor=-1",
        "teacher.effectiveCursor=8",
    ).replace(
        "teacher.portraitVisible=0",
        "teacher.portraitVisible=1",
    )

    rec_later_curr_raw = rec_later_base_raw.replace(
        "teacher.currentCursor=8",
        "teacher.currentCursor=9",
    ).replace(
        "teacher.effectiveCursor=8",
        "teacher.effectiveCursor=9",
    ).replace(
        "student.currentCursor=-1",
        "student.currentCursor=1",
    ).replace(
        "student.effectiveCursor=-1",
        "student.effectiveCursor=1",
    ).replace(
        "student.currentEnable=0",
        "student.currentEnable=1",
    ).replace(
        "student.portraitVisible=0",
        "student.portraitVisible=1",
    ).replace(
        "student.startDueFrame=440",
        "student.startDueFrame=570",
    )
    rec_later_curr = _make_record(
        sample_index=35,
        elapsed_ms=48,
        raw=rec_later_curr_raw,
        parsed=parse_stage1ovl_rail(rec_later_curr_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 148},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_later_curr["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    rec_later_curr["focus"]["producer"]["studentPreviewSeedSourceKind"] = 4
    rec_later_curr["focus"]["producer"]["studentLiveTimingSelectedSourceKind"] = 5
    rec_later_curr["focus"]["producer"]["studentCurrentLaneMaterializationAllowed"] = True
    rec_later_curr["focus"]["live"]["student"]["startDueFrame"] = 570

    rec_later8_hits = _update_markers(later_cycle_summary, rec_second_curr, rec_later_curr)
    assert "later_cycle_teacher8_student_hidden" in rec_later8_hits

    later_cycle_inferred_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 10},
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": {"sample_index": 30},
            "second_overlap_teacher1_student9": {"sample_index": 31},
            "second_overlap_teacher2_student_hidden": {"sample_index": 32},
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": {"sample_index": 33},
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_later_hidden_raw = rec_later_curr_raw.replace(
        "teacher.currentCursor=9",
        "teacher.currentCursor=-1",
    ).replace(
        "teacher.effectiveCursor=9",
        "teacher.effectiveCursor=-1",
    ).replace(
        "teacher.currentEnable=1",
        "teacher.currentEnable=0",
    ).replace(
        "teacher.portraitVisible=1",
        "teacher.portraitVisible=0",
    ).replace(
        "teacher.startDueFrame=504",
        "teacher.startDueFrame=0",
    ).replace(
        "student.currentCursor=1",
        "student.currentCursor=2",
    ).replace(
        "student.effectiveCursor=1",
        "student.effectiveCursor=2",
    )
    rec_later_hidden = _make_record(
        sample_index=36,
        elapsed_ms=56,
        raw=rec_later_hidden_raw,
        parsed=parse_stage1ovl_rail(rec_later_hidden_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 156},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_later_hidden["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    rec_later_hidden["focus"]["producer"]["studentPreviewSeedSourceKind"] = 4
    rec_later_hidden["focus"]["producer"]["studentLiveTimingSelectedSourceKind"] = 5
    rec_later_hidden["focus"]["producer"]["studentCurrentLaneMaterializationAllowed"] = True
    rec_later_hidden["focus"]["live"]["student"]["startDueFrame"] = 570

    later9_hits = _update_markers(
        later_cycle_summary,
        rec_later_curr,
        rec_later_hidden,
    )
    assert "later_cycle_teacher9_student1" in later9_hits

    later8_inferred_hits = _update_markers(
        later_cycle_inferred_summary, rec_second_curr, rec_later_hidden
    )
    assert "later_cycle_teacher8_student_hidden" in later8_inferred_hits
    assert later_cycle_inferred_summary["windows"][
        "later_cycle_teacher8_student_hidden"
    ]["reason"] == "later_cycle_bootstrap_entry_inferred_from_hidden_student"
    assert "later_cycle_teacher9_student1" in later8_inferred_hits

    rec_later_teacher1_student9_raw = rec_later_curr_raw.replace(
        "teacher.currentCursor=9",
        "teacher.currentCursor=1",
    ).replace(
        "teacher.effectiveCursor=9",
        "teacher.effectiveCursor=1",
    ).replace(
        "student.currentCursor=1",
        "student.currentCursor=9",
    ).replace(
        "student.effectiveCursor=1",
        "student.effectiveCursor=9",
    )
    rec_later_teacher1_student9 = _make_record(
        sample_index=37,
        elapsed_ms=64,
        raw=rec_later_teacher1_student9_raw,
        parsed=parse_stage1ovl_rail(rec_later_teacher1_student9_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 164},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_later_teacher1_student9["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    rec_later_teacher1_student9["focus"]["producer"]["studentPreviewSeedSourceKind"] = 4
    rec_later_teacher1_student9["focus"]["producer"]["studentLiveTimingSelectedSourceKind"] = 5
    rec_later_teacher1_student9["focus"]["producer"]["studentCurrentLaneMaterializationAllowed"] = True
    rec_later_teacher1_student9["focus"]["live"]["student"]["startDueFrame"] = 570

    later_hidden_hits = _update_markers(
        later_cycle_summary,
        rec_later_hidden,
        rec_later_teacher1_student9,
    )
    assert "later_cycle_teacher_hidden_student2" in later_hidden_hits

    rec_later_teacher2_hidden_raw = rec_later_teacher1_student9_raw.replace(
        "teacher.currentCursor=1",
        "teacher.currentCursor=2",
    ).replace(
        "teacher.effectiveCursor=1",
        "teacher.effectiveCursor=2",
    ).replace(
        "student.currentCursor=9",
        "student.currentCursor=-1",
    ).replace(
        "student.effectiveCursor=9",
        "student.effectiveCursor=-1",
    ).replace(
        "student.currentEnable=1",
        "student.currentEnable=0",
    ).replace(
        "student.portraitVisible=1",
        "student.portraitVisible=0",
    )
    rec_later_teacher2_hidden = _make_record(
        sample_index=36,
        elapsed_ms=56,
        raw=rec_later_teacher2_hidden_raw,
        parsed=parse_stage1ovl_rail(rec_later_teacher2_hidden_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 156},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_later_teacher2_hidden["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    later_hidden_inferred_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": {"sample_index": 1},
            "first_teacher_end": {"sample_index": 2},
            "second_teacher_start": {"sample_index": 30},
            "first_overlap_teacher8_student_hidden": {"sample_index": 20},
            "first_overlap_teacher9_student1": {"sample_index": 21},
            "first_overlap_teacher_hidden_student2": {"sample_index": 22},
            "second_overlap_teacher_hidden_student8": {"sample_index": 30},
            "second_overlap_teacher1_student9": {"sample_index": 31},
            "second_overlap_teacher2_student_hidden": {"sample_index": 32},
            "current_lane_release_wait_tail": {"sample_index": 33},
            "preview_hold_after_takeover": {"sample_index": 34},
            "later_cycle_teacher8_student_hidden": {"sample_index": 35},
            "later_cycle_teacher9_student1": {"sample_index": 36},
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    later_hidden_inferred_hits = _update_markers(
        later_hidden_inferred_summary,
        rec_later_hidden,
        rec_later_teacher2_hidden,
    )
    assert "later_cycle_teacher_hidden_student2" in later_hidden_inferred_hits

    first_overlap_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_first_prev_raw = raw.replace(
        "teacher.currentCursor=2",
        "teacher.currentCursor=8",
    ).replace(
        "teacher.effectiveCursor=2",
        "teacher.effectiveCursor=8",
    ).replace(
        "student.currentCursor=4",
        "student.currentCursor=-1",
    ).replace(
        "student.effectiveCursor=4",
        "student.effectiveCursor=-1",
    ).replace(
        "student.currentEnable=1",
        "student.currentEnable=0",
    ).replace(
        "student.portraitVisible=1",
        "student.portraitVisible=0",
    ).replace(
        "teacher.startDueFrame=410",
        "teacher.startDueFrame=384",
    )
    rec_first_prev = _make_record(
        sample_index=20,
        elapsed_ms=32,
        raw=rec_first_prev_raw,
        parsed=parse_stage1ovl_rail(rec_first_prev_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 132},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_first_curr_raw = rec_first_prev_raw.replace(
        "teacher.currentCursor=8",
        "teacher.currentCursor=9",
    ).replace(
        "teacher.effectiveCursor=8",
        "teacher.effectiveCursor=9",
    ).replace(
        "student.currentCursor=-1",
        "student.currentCursor=1",
    ).replace(
        "student.effectiveCursor=-1",
        "student.effectiveCursor=1",
    ).replace(
        "student.currentEnable=0",
        "student.currentEnable=1",
    ).replace(
        "student.portraitVisible=0",
        "student.portraitVisible=1",
    ).replace(
        "student.startDueFrame=420",
        "student.startDueFrame=450",
    )
    rec_first_curr = _make_record(
        sample_index=21,
        elapsed_ms=40,
        raw=rec_first_curr_raw,
        parsed=parse_stage1ovl_rail(rec_first_curr_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 140},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    first_hits = _update_markers(first_overlap_summary, rec_first_prev, rec_first_curr)
    assert "first_overlap_teacher8_student_hidden" in first_hits
    assert "first_overlap_teacher9_student1" in first_hits

    rec_first_prev_teacher7_raw = rec_first_prev_raw.replace(
        "teacher.currentCursor=8",
        "teacher.currentCursor=7",
    ).replace(
        "teacher.effectiveCursor=8",
        "teacher.effectiveCursor=7",
    )
    rec_first_prev_teacher7 = _make_record(
        sample_index=20,
        elapsed_ms=32,
        raw=rec_first_prev_teacher7_raw,
        parsed=parse_stage1ovl_rail(rec_first_prev_teacher7_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 132},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    first_teacher7_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    first_teacher7_hits = _update_markers(
        first_teacher7_summary, rec_first_prev_teacher7, rec_first_curr
    )
    assert "first_overlap_teacher8_student_hidden" in first_teacher7_hits
    assert "first_overlap_teacher9_student1" in first_teacher7_hits

    rec_first_prev_teacher9_hidden_raw = rec_first_prev_raw.replace(
        "teacher.currentCursor=8",
        "teacher.currentCursor=9",
    ).replace(
        "teacher.effectiveCursor=8",
        "teacher.effectiveCursor=9",
    )
    rec_first_prev_teacher9_hidden = _make_record(
        sample_index=20,
        elapsed_ms=32,
        raw=rec_first_prev_teacher9_hidden_raw,
        parsed=parse_stage1ovl_rail(rec_first_prev_teacher9_hidden_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 132},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_first_hidden_student2_raw = rec_first_prev_teacher9_hidden_raw.replace(
        "teacher.currentCursor=9",
        "teacher.currentCursor=-1",
    ).replace(
        "teacher.effectiveCursor=9",
        "teacher.effectiveCursor=-1",
    ).replace(
        "teacher.currentEnable=1",
        "teacher.currentEnable=0",
    ).replace(
        "teacher.portraitVisible=1",
        "teacher.portraitVisible=0",
    ).replace(
        "student.currentCursor=-1",
        "student.currentCursor=2",
    ).replace(
        "student.effectiveCursor=-1",
        "student.effectiveCursor=2",
    ).replace(
        "student.currentEnable=0",
        "student.currentEnable=1",
    ).replace(
        "student.portraitVisible=0",
        "student.portraitVisible=1",
    ).replace(
        "student.startDueFrame=420",
        "student.startDueFrame=450",
    )
    rec_first_hidden_student2 = _make_record(
        sample_index=21,
        elapsed_ms=40,
        raw=rec_first_hidden_student2_raw,
        parsed=parse_stage1ovl_rail(rec_first_hidden_student2_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 140},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    first_hidden2_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    first_hidden2_hits = _update_markers(
        first_hidden2_summary,
        rec_first_prev_teacher9_hidden,
        rec_first_hidden_student2,
    )
    assert "first_overlap_teacher8_student_hidden" in first_hidden2_hits
    assert "first_overlap_teacher9_student1" in first_hidden2_hits
    assert "first_overlap_teacher_hidden_student2" in first_hidden2_hits

    rec_first_hidden_student1_raw = rec_first_hidden_student2_raw.replace(
        "student.currentCursor=2",
        "student.currentCursor=1",
    ).replace(
        "student.effectiveCursor=2",
        "student.effectiveCursor=1",
    )
    rec_first_hidden_student1 = _make_record(
        sample_index=21,
        elapsed_ms=40,
        raw=rec_first_hidden_student1_raw,
        parsed=parse_stage1ovl_rail(rec_first_hidden_student1_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 140},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    first_hidden1_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    first_hidden1_hits = _update_markers(
        first_hidden1_summary,
        rec_first_prev_teacher9_hidden,
        rec_first_hidden_student1,
    )
    assert "first_overlap_teacher8_student_hidden" in first_hidden1_hits
    assert "first_overlap_teacher9_student1" in first_hidden1_hits

    release_hidden_summary = {
        "windows": {
            "first_teacher_start": {"sample_index": 0},
            "first_student_start": None,
            "first_teacher_end": {"sample_index": 1},
            "second_teacher_start": {"sample_index": 30},
            "first_overlap_teacher8_student_hidden": {"sample_index": 20},
            "first_overlap_teacher9_student1": {"sample_index": 21},
            "first_overlap_teacher_hidden_student2": {"sample_index": 22},
            "second_overlap_teacher_hidden_student8": {"sample_index": 30},
            "second_overlap_teacher1_student9": {"sample_index": 30},
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
        }
    }
    rec_release_hidden_raw = rec_second_curr_raw.replace(
        "teacher.currentCursor=1",
        "teacher.currentCursor=3",
    ).replace(
        "teacher.effectiveCursor=1",
        "teacher.effectiveCursor=3",
    ).replace(
        "student.currentCursor=9",
        "student.currentCursor=-1",
    ).replace(
        "student.effectiveCursor=9",
        "student.effectiveCursor=-1",
    ).replace(
        "student.portraitVisible=1",
        "student.portraitVisible=0",
    )
    rec_release_hidden = _make_record(
        sample_index=31,
        elapsed_ms=32,
        raw=rec_release_hidden_raw,
        parsed=parse_stage1ovl_rail(rec_release_hidden_raw),
        status={"scene": 1, "stageRunning": 1, "stageFrame": 132},
        timing={"rail_query_ms": 1, "status_query_ms": 0, "loop_ms": 1},
        status_fresh=False,
    )
    rec_release_hidden["focus"]["producer"]["teacherSource"] = {
        "name": "preview",
        "value": 2,
    }
    release_hidden_hits = _update_markers(
        release_hidden_summary, rec_release, rec_release_hidden
    )
    assert "current_lane_release_wait_tail" in release_hidden_hits

    stop_record = rec_second_curr
    stop_summary = {
        "samples_captured": 1,
        "windows": {
            "second_teacher_start": {"queryFrame": 432},
        },
    }
    stop_args = argparse.Namespace(
        tail_query_frames=24,
        max_query_frame=999,
        max_stage_frame=999,
        seconds=0.0,
        stop_on_second_tail=True,
    )
    assert _stop_reason(stop_args, stop_record, rec_second_prev, stop_summary) == "second_teacher_tail_complete"
    no_tail_stop_args = argparse.Namespace(
        tail_query_frames=24,
        max_query_frame=999,
        max_stage_frame=999,
        seconds=0.0,
        stop_on_second_tail=False,
    )
    assert _stop_reason(no_tail_stop_args, stop_record, rec_second_prev, stop_summary) is None

    print("[SELFTEST] ok")
    return 0


def capture(args: argparse.Namespace) -> int:
    output_dir = _unique_output_dir(args.output_dir, args.label)
    timeline_path = output_dir / "timeline.jsonl"
    keyframes_path = output_dir / "keyframes.json"
    windows_path = output_dir / "windows.json"
    session_path = output_dir / "session.json"
    summary_path = output_dir / "summary.txt"

    tester = DispatcherTester()
    if not tester.check_connection():
        if not tester._restart_game_and_wait_debug_ready(15.0):
            print("[FAIL] Win debug connection unavailable")
            return 1
    if not _boot_stage1(tester, args.boot_mode):
        print("[FAIL] could not boot Stage1")
        return 1

    summary = {
        "samples_captured": 0,
        "runtime_available_samples": 0,
        "parse_ok_samples": 0,
        "status_refreshes": 0,
        "rail_query_ms_total": 0.0,
        "max_rail_query_ms": 0,
        "loop_ms_total": 0.0,
        "row_modes_seen": set(),
        "lesson_selectors_seen": set(),
        "teacher_sources_seen": set(),
        "windows": {
            "first_teacher_start": None,
            "first_student_start": None,
            "first_teacher_end": None,
            "second_teacher_start": None,
            "first_overlap_teacher8_student_hidden": None,
            "first_overlap_teacher9_student1": None,
            "first_overlap_teacher_hidden_student2": None,
            "second_overlap_teacher_hidden_student8": None,
            "second_overlap_teacher1_student9": None,
            "second_overlap_teacher2_student_hidden": None,
            "current_lane_release_wait_tail": None,
            "preview_hold_after_takeover": None,
            "later_cycle_teacher8_student_hidden": None,
            "later_cycle_teacher9_student1": None,
            "later_cycle_teacher_hidden_student2": None,
            "late_repeat_teacher_hidden_student8": None,
            "late_repeat_teacher1_student9": None,
            "late_repeat_teacher2_student_hidden": None,
            "late_repeat_preview_teacher9_student1": None,
            "late_repeat_preview_teacher_hidden_student2": None,
            "late_repeat_preview_teacher_hidden_student8": None,
            "late_repeat_second_teacher1_student9": None,
            "late_repeat_second_teacher2_student_hidden": None,
        },
    }
    records: list[dict[str, Any]] = []
    keyframes: list[dict[str, Any]] = []
    last_status: dict[str, Any] = {}
    start_time = time.monotonic()
    prev_record: dict[str, Any] | None = None
    stop_reason = "not_started"

    sample_index = 0
    while True:
        loop_start = time.monotonic()
        rail_start = time.monotonic()
        raw = tester.client.send_full("stage1ovl rail")
        rail_query_ms = int((time.monotonic() - rail_start) * 1000.0)
        parsed = parse_stage1ovl_rail(raw)

        status_query_ms = 0
        status_fresh = False
        if sample_index == 0 or (args.status_every > 0 and sample_index % args.status_every == 0):
            status_start = time.monotonic()
            last_status = _extract_status_subset(tester.client.json_status())
            status_query_ms = int((time.monotonic() - status_start) * 1000.0)
            status_fresh = True
            summary["status_refreshes"] += 1

        elapsed_ms = int((time.monotonic() - start_time) * 1000.0)
        loop_ms = int((time.monotonic() - loop_start) * 1000.0)
        record = _make_record(
            sample_index=sample_index,
            elapsed_ms=elapsed_ms,
            raw=raw,
            parsed=parsed,
            status=last_status,
            timing={
                "rail_query_ms": rail_query_ms,
                "status_query_ms": status_query_ms,
                "loop_ms": loop_ms,
            },
            status_fresh=status_fresh,
        )
        record["change_tags"] = _change_tags(prev_record, record)
        markers_hit = _update_markers(summary, prev_record, record)
        if markers_hit:
            record["window_hits"] = markers_hit
            for marker_name in markers_hit:
                tag = f"window_{marker_name}"
                if tag not in record["change_tags"]:
                    record["change_tags"].append(tag)
        else:
            record["window_hits"] = []

        records.append(record)
        _write_jsonl_line(timeline_path, record)

        summary["samples_captured"] += 1
        summary["runtime_available_samples"] += int(parsed.get("runtime_available") is True)
        summary["parse_ok_samples"] += int(parsed.get("parse_ok") is True)
        summary["rail_query_ms_total"] += rail_query_ms
        summary["max_rail_query_ms"] = max(summary["max_rail_query_ms"], rail_query_ms)
        summary["loop_ms_total"] += loop_ms

        rail_state = parsed.get("rail") or {}
        display_gate = parsed.get("displayGate") or {}
        producer = ((parsed.get("cursor") or {}).get("producer") or {})
        row_mode = rail_state.get("rowMode")
        lesson_selector = display_gate.get("lessonSelector")
        teacher_source = (producer.get("teacherSource") or {}).get("name")
        if row_mode is not None:
            summary["row_modes_seen"].add(int(row_mode))
        if lesson_selector is not None:
            summary["lesson_selectors_seen"].add(int(lesson_selector))
        if teacher_source is not None:
            summary["teacher_sources_seen"].add(str(teacher_source))

        if record["change_tags"]:
            keyframes.append(
                {
                    "sample_index": record["sample_index"],
                    "elapsed_ms": record["elapsed_ms"],
                    "stageFrame": (record["stage_status"] or {}).get("stageFrame"),
                    "queryFrame": rail_state.get("queryFrame"),
                    "change_tags": record["change_tags"],
                    "window_hits": record["window_hits"],
                    "focus": record["focus"],
                }
            )

        stop_reason = _stop_reason(args, record, prev_record, summary)
        prev_record = record
        sample_index += 1
        if stop_reason is not None:
            break

        sleep_sec = max(0.0, args.sample_ms / 1000.0 - (time.monotonic() - loop_start))
        if sleep_sec > 0:
            time.sleep(sleep_sec)

    summary = _finalize_summary(summary)
    windows_payload = {
        "api": "openai-responses",
        "capture_kind": CAPTURE_KIND,
        "captured_at": _dt.datetime.now().isoformat(),
        "label": args.label,
        "windows": {
            name: _extract_window(records, marker, args.pre_samples, args.post_samples)
            for name, marker in summary["windows"].items()
        },
    }
    session_payload = {
        "api": "openai-responses",
        "capture_kind": CAPTURE_KIND,
        "captured_at": _dt.datetime.now().isoformat(),
        "label": args.label,
        "boot_mode": args.boot_mode,
        "sample_ms": args.sample_ms,
        "status_every": args.status_every,
        "max_query_frame": args.max_query_frame,
        "max_stage_frame": args.max_stage_frame,
        "tail_query_frames": args.tail_query_frames,
        "seconds": args.seconds,
        "stop_on_second_tail": bool(getattr(args, "stop_on_second_tail", True)),
        "stop_reason": stop_reason,
        "summary": summary,
        "files": {
            "timeline": str(timeline_path),
            "keyframes": str(keyframes_path),
            "windows": str(windows_path),
            "session": str(session_path),
            "summary": str(summary_path),
        },
    }

    _write_json(keyframes_path, keyframes)
    _write_json(windows_path, windows_payload)
    _write_json(session_path, session_payload)
    summary_path.write_text(_summary_text(output_dir, session_payload), encoding="utf-8")

    print(f"[OK] wrote {output_dir}")
    return 0 if summary["samples_captured"] > 0 else 1


def build_argparser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Win-only high-frequency Stage1 teacher rail capture")
    parser.add_argument("--boot-mode", choices=("direct", "stage-select", "attach"), default="direct")
    parser.add_argument("--sample-ms", type=int, default=8)
    parser.add_argument(
        "--status-every",
        type=int,
        default=6,
        help="Refresh json status every N samples; 0 disables periodic refresh after sample 0",
    )
    parser.add_argument("--max-query-frame", type=int, default=360)
    parser.add_argument("--max-stage-frame", type=int, default=360)
    parser.add_argument("--tail-query-frames", type=int, default=24)
    parser.set_defaults(stop_on_second_tail=True)
    parser.add_argument(
        "--stop-on-second-tail",
        dest="stop_on_second_tail",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    parser.add_argument(
        "--no-stop-on-second-tail",
        dest="stop_on_second_tail",
        action="store_false",
        help="Continue sampling past second teacher tail until another stop condition is reached",
    )
    parser.add_argument("--seconds", type=float, default=20.0)
    parser.add_argument("--pre-samples", type=int, default=6)
    parser.add_argument("--post-samples", type=int, default=6)
    parser.add_argument("--label", default="direct_boot_teacher_rail")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--self-test", action="store_true")
    return parser


def main() -> int:
    parser = build_argparser()
    args = parser.parse_args()
    if args.self_test:
        return _self_test()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    return capture(args)


if __name__ == "__main__":
    raise SystemExit(main())
