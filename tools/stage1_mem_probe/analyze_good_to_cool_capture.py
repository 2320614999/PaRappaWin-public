#!/usr/bin/env python3
"""Summarize external Stage1 GOOD->COOL capture evidence.

This is a detachable analysis helper for JSONL produced by stage1_mem_probe.py.
It does not attach to the game and does not modify runtime state.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path
from typing import Any


BASELINES: dict[int, dict[str, Any]] = {
    8: {
        "psx_frame": 1157,
        "purpose": "first GOOD-row default branch, accepted=0",
        "sig": {
            "descriptor.lessonId": 1,
            "descriptor.anchorSlotIndex": 8,
            "descriptor.requiredClassToken": 1,
            "descriptor.penaltyWeight": 1,
            "descriptor.finalResolutionThreshold": 95,
            "descriptor.requiredMask": 0x10,
            "descriptor.defaultLookbackPageCount": 1,
            "descriptor.defaultFlagWord": 0x0002,
        },
        "expect": {
            "runtime.rightRankActiveRow": 1,
            "runtime.word9182AEnabled": 1,
            "acceptedProducer.acceptedContributionCount": 0,
            "acceptedProducer.recordedSplitCount": 0,
            "acceptedProducer.penaltySideSplitCount": 0,
            "bucket30.gameplayPairBonus": 0,
        },
    },
    9: {
        "psx_frame": 1287,
        "purpose": "second GOOD-row baseline, 80014A80 called but accepted=0",
        "sig": {
            "descriptor.lessonId": 1,
            "descriptor.anchorSlotIndex": 8,
            "descriptor.requiredClassToken": 2,
            "descriptor.penaltyWeight": 1,
            "descriptor.finalResolutionThreshold": 95,
            "descriptor.requiredMask": 0x20,
            "descriptor.defaultLookbackPageCount": 1,
            "descriptor.defaultFlagWord": 0x000E,
        },
        "expect": {
            "runtime.rightRankActiveRow": 1,
            "runtime.word9182AEnabled": 1,
            "acceptedProducer.acceptedContributionCount": 0,
            "acceptedProducer.recordedSplitCount": 0,
            "acceptedProducer.penaltySideSplitCount": 0,
            "bucket30.gameplayPairBonus": 0,
        },
    },
    25: {
        "psx_frame": 3375,
        "purpose": "occupied page slots, no pair bonus, active rank 2",
        "sig": {
            "descriptor.anchorSlotIndex": 8,
            "descriptor.requiredClassToken": 7,
            "descriptor.finalResolutionThreshold": 95,
            "descriptor.requiredMask": 0x8,
            "descriptor.defaultLookbackPageCount": 1,
            "descriptor.defaultFlagWord": 0x000E,
        },
        "expect": {
            "runtime.rightRankActiveRow": 2,
            "runtime.word9182AEnabled": 1,
            "acceptedProducer.acceptedContributionCount": 3,
            "acceptedProducer.recordedSplitCount": 2,
            "acceptedProducer.penaltySideSplitCount": 1,
            "bucket30.gameplayReaderPageOrdinal": 24,
            "bucket30.gameplayPairBonus": 0,
            "bucket30.gameplaySpillPenalty": -1,
            "bucket30.gameplayDescriptorSubdelta": 4,
        },
        "slots": {7: (0x8, 7, 1), 9: (0x8, 7, 1)},
    },
    37: {
        "psx_frame": 4941,
        "purpose": "side-effect control sample, not a direct pair-bonus target",
        "sig": {
            "descriptor.anchorSlotIndex": 0,
            "descriptor.requiredClassToken": 5,
            "descriptor.finalResolutionThreshold": 117,
            "descriptor.requiredMask": 0x4,
            "descriptor.defaultLookbackPageCount": 1,
            "descriptor.defaultFlagWord": 0x0002,
        },
        "expect": {
            "runtime.word9182AEnabled": 1,
            "acceptedProducer.acceptedContributionCount": 0,
        },
    },
    39: {
        "psx_frame": 5203,
        "purpose": "later GOOD-row accepted records, positive commit, no pair bonus",
        "sig": {
            "descriptor.anchorSlotIndex": 0,
            "descriptor.requiredClassToken": 7,
            "descriptor.finalResolutionThreshold": 117,
            "descriptor.requiredMask": 0x8,
            "descriptor.defaultLookbackPageCount": 1,
            "descriptor.defaultFlagWord": 0x0006,
        },
        "expect": {
            "runtime.rightRankActiveRow": 1,
            "runtime.word9182AEnabled": 1,
            "acceptedProducer.acceptedContributionCount": 2,
            "acceptedProducer.recordedSplitCount": 2,
            "acceptedProducer.penaltySideSplitCount": 0,
            "bucket30.gameplayReaderPageOrdinal": 38,
            "bucket30.gameplayPairBonus": 0,
            "bucket30.gameplaySpillPenalty": 0,
            "bucket30.gameplayDescriptorSubdelta": 6,
        },
        "slots": {0: (0x8, 7, 1), 4: (0x8, 7, 1)},
    },
}


IMPORTANT_KEYS = [
    "runtime.queryFrame",
    "runtime.rightRankActiveRow",
    "runtime.word9182AEnabled",
    "runtime.descriptorSubstate50",
    "runtime.ctx52ReplayMode7A60",
    "runtime.runnerPreambleHalfWindow34Known",
    "runtime.runnerPreambleHalfWindow34",
    "descriptorCadence.currentDescriptor40Available",
    "descriptorCadence.currentDescriptor40RowIndex",
    "acceptedCarrier.acceptedTick96Known",
    "acceptedCarrier.acceptedTick96",
    "acceptedCarrier.writePageOrdinal38",
    "acceptedCarrier.recordSlot24",
    "acceptedProducer.acceptedContributionCount",
    "acceptedProducer.recordedSplitCount",
    "acceptedProducer.penaltySideSplitCount",
    "scorer.acceptedCount91810",
    "scorer.recordedHitCount91812",
    "scorer.penaltySideCount91814",
    "scorer.accumulator91816",
    "bucket30.scorerCommitWindowOpen",
    "bucket30.gameplayCurrentBranchKnown",
    "bucket30.gameplayCurrentBranchSpecial",
    "bucket30.gameplayAcceptedCountKnown",
    "bucket30.gameplayAcceptedCount",
    "bucket30.gameplayLookbackPageCountKnown",
    "bucket30.gameplayLookbackPageCount",
    "bucket30.gameplayDescriptorSubdeltaKnown",
    "bucket30.gameplayDescriptorSubdelta",
    "bucket30.gameplayReaderPageOrdinalKnown",
    "bucket30.gameplayReaderPageOrdinal",
    "bucket30.gameplayRequiredMaskKnown",
    "bucket30.gameplayRequiredMask",
    "bucket30.gameplayUnionMaskKnown",
    "bucket30.gameplayUnionMask",
    "bucket30.gameplayAnchorSlotIndexKnown",
    "bucket30.gameplayAnchorSlotIndex",
    "bucket30.gameplayRequiredClassTokenKnown",
    "bucket30.gameplayRequiredClassToken",
    "bucket30.gameplayAnchorSlotOccupiedKnown",
    "bucket30.gameplayAnchorSlotOccupied",
    "bucket30.gameplayAnchorSlotClassTokenKnown",
    "bucket30.gameplayAnchorSlotClassToken",
    "bucket30.gameplayAnchorClassMatchKnown",
    "bucket30.gameplayAnchorClassMatch",
    "bucket30.gameplayPairBonusKnown",
    "bucket30.gameplayPairBonus",
    "bucket30.gameplaySpillPenaltyKnown",
    "bucket30.gameplaySpillPenalty",
    "bucket30.gameplayAdditiveTermKnown",
    "bucket30.gameplayAdditiveTerm",
    "bucket30.gameplayCommitTermKnown",
    "bucket30.gameplayCommitTerm",
    "bucket30.gameplayScoreWritebackKnown",
    "bucket30.gameplayScoreWriteback",
    "bucket30.phase1Delta91816MinusPrev18",
    "bucket30.formalWritebackKnown",
    "bucket30.formalWritebackValue",
    "bucket30.resolutionCalled",
    "bucket30.resolutionKnown",
    "bucket30.resolutionInputPhase1Classifier36",
    "bucket30.tieBreakerResult14548",
    "bucket30.resolutionV22",
    "bucket30.goodToCoolCommitted",
]


def load_jsonl(path: Path) -> list[dict[str, Any]]:
    samples: list[dict[str, Any]] = []
    with path.open("r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                samples.append(json.loads(line))
            except json.JSONDecodeError as exc:
                raise SystemExit(f"{path}:{line_no}: {exc}") from exc
    return samples


def mem(sample: dict[str, Any]) -> dict[str, Any]:
    return sample.get("mem") or {}


def v(sample: dict[str, Any], key: str, default: Any = None) -> Any:
    return mem(sample).get(key, default)


def descriptor_signature_matches(sample: dict[str, Any], row: int) -> bool:
    m = mem(sample)
    if m.get("descriptorCadence.currentDescriptor40Available") and (
        m.get("descriptorCadence.currentDescriptor40RowIndex") == row
    ):
        return True
    sig = BASELINES[row]["sig"]
    return all(m.get(key) == expected for key, expected in sig.items())


def row_candidates(samples: list[dict[str, Any]], row: int) -> list[dict[str, Any]]:
    return [sample for sample in samples if descriptor_signature_matches(sample, row)]


def selected_sample(candidates: list[dict[str, Any]]) -> dict[str, Any] | None:
    if not candidates:
        return None
    for sample in reversed(candidates):
        if (
            v(sample, "bucket30.gameplayScoreWritebackKnown")
            or v(sample, "bucket30.gameplayPairBonusKnown")
            or v(sample, "bucket30.gameplayDescriptorSubdeltaKnown")
        ):
            return sample
    return candidates[-1]


def unique_values(candidates: list[dict[str, Any]], key: str, limit: int = 8) -> list[Any]:
    out: list[Any] = []
    seen: set[str] = set()
    for sample in candidates:
        value = v(sample, key)
        marker = repr(value)
        if marker in seen:
            continue
        seen.add(marker)
        out.append(value)
        if len(out) >= limit:
            break
    return out


def decode_page_slots(raw_hex: Any, page_ordinal: Any) -> dict[int, tuple[int, int, int]]:
    if not isinstance(raw_hex, str) or page_ordinal is None:
        return {}
    try:
        data = bytes.fromhex(raw_hex)
        page_index = int(page_ordinal) & 3
    except Exception:
        return {}
    page_offset = page_index * 384
    if page_offset + 384 > len(data):
        return {}
    slots: dict[int, tuple[int, int, int]] = {}
    for slot in range(32):
        offset = page_offset + slot * 12
        mask = struct.unpack_from("<I", data, offset)[0]
        class_token = struct.unpack_from("<H", data, offset + 4)[0]
        occupied = struct.unpack_from("<H", data, offset + 6)[0]
        if mask or class_token or occupied:
            slots[slot] = (mask, class_token, occupied)
    return slots


def compact_slots(slots: dict[int, tuple[int, int, int]]) -> str:
    if not slots:
        return "{}"
    parts = [
        f"{slot}:mask=0x{mask:X},cls={cls},occ={occ}"
        for slot, (mask, cls, occ) in sorted(slots.items())
    ]
    return "{" + "; ".join(parts) + "}"


def compare_expected(sample: dict[str, Any], row: int) -> list[str]:
    problems: list[str] = []
    for key, expected in BASELINES[row].get("expect", {}).items():
        actual = v(sample, key)
        if actual != expected:
            problems.append(f"{key}: win={actual!r} psx={expected!r}")
    expected_slots = BASELINES[row].get("slots")
    if expected_slots:
        reader = v(sample, "bucket30.gameplayReaderPageOrdinal")
        actual_slots = decode_page_slots(v(sample, "pageMirror.rawPages"), reader)
        for slot, expected in expected_slots.items():
            actual = actual_slots.get(slot)
            if actual != expected:
                problems.append(f"slot{slot}: win={actual!r} psx={expected!r}")
    return problems


def print_row(samples: list[dict[str, Any]], row: int) -> None:
    base = BASELINES[row]
    candidates = row_candidates(samples, row)
    print(f"\n## row {row} / PSX frame {base['psx_frame']}")
    print(f"purpose: {base['purpose']}")
    print(f"samples: {len(candidates)}")
    if not candidates:
        print("status: missing in capture")
        return
    q_first = v(candidates[0], "runtime.queryFrame")
    q_last = v(candidates[-1], "runtime.queryFrame")
    print(f"queryFrame: {q_first}..{q_last}")
    sample = selected_sample(candidates)
    assert sample is not None
    print("selected:")
    for key in IMPORTANT_KEYS:
        value = v(sample, key)
        if value is not None:
            print(f"  {key}={value}")
    reader = v(sample, "bucket30.gameplayReaderPageOrdinal")
    slots = decode_page_slots(v(sample, "pageMirror.rawPages"), reader)
    if slots:
        print(f"  pageMirror.readerSlots={compact_slots(slots)}")
    print("ranges:")
    for key in (
        "runtime.word9182AEnabled",
        "runtime.rightRankActiveRow",
        "scorer.accumulator91816",
        "bucket30.gameplayPairBonusKnown",
        "bucket30.gameplayPairBonus",
        "bucket30.gameplayDescriptorSubdelta",
        "bucket30.gameplayCommitTerm",
        "bucket30.gameplayScoreWriteback",
        "bucket30.resolutionV22",
        "bucket30.goodToCoolCommitted",
    ):
        vals = unique_values(candidates, key)
        print(f"  {key}={vals}")
    problems = compare_expected(sample, row)
    if problems:
        print("psx-baseline differences:")
        for item in problems:
            print(f"  - {item}")
    else:
        print("psx-baseline differences: none on selected sample")


def decision_hints(samples: list[dict[str, Any]]) -> None:
    print("\n## decision hints")
    rows = {row: row_candidates(samples, row) for row in BASELINES}
    gate_bad = []
    for row in (8, 9):
        cands = rows[row]
        if cands and any(v(s, "runtime.word9182AEnabled") == 0 for s in cands):
            gate_bad.append(row)
    if gate_bad:
        print(
            "- word9182AEnabled dropped on GOOD baseline rows "
            f"{gate_bad}: target setup propagation or unintended 800143F0(0) clear."
        )
    else:
        print("- word9182AEnabled did not prove false on rows 8/9.")

    row9 = rows[9]
    if row9:
        pair_known = unique_values(row9, "bucket30.gameplayPairBonusKnown")
        if pair_known and all(not x for x in pair_known):
            print(
                "- row9 did not mark pair-bonus evaluation: target 80014D58 "
                "branch-call condition carrier."
            )
    row25 = selected_sample(rows[25])
    row39 = selected_sample(rows[39])
    for row, sample in ((25, row25), (39, row39)):
        if sample is None:
            continue
        slot_diffs = [
            p
            for p in compare_expected(sample, row)
            if p.startswith("slot")
            or p.startswith("bucket30.gameplayReaderPageOrdinal")
        ]
        if slot_diffs:
            print(
                f"- row{row} page/slot baseline differs: target 80014614 "
                "page write, page ordinal, or pageMirror ring mapping."
            )
            for diff in slot_diffs[:6]:
                print(f"  {diff}")
    if any(
        selected_sample(rows[row]) is not None
        and v(selected_sample(rows[row]) or {}, "bucket30.gameplayCommitTermKnown")
        and v(selected_sample(rows[row]) or {}, "bucket30.gameplayCommitTerm")
        == v(selected_sample(rows[row]) or {}, "bucket30.gameplayDescriptorSubdelta")
        and v(selected_sample(rows[row]) or {}, "scorer.accumulator91816")
        != v(selected_sample(rows[row]) or {}, "bucket30.gameplayScoreWriteback")
        for row in BASELINES
    ):
        print(
            "- commit term/subdelta is present but scorer/writeback diverges: "
            "target 80014D58 writeback or scorerPort accumulator propagation."
        )
    if any(
        selected_sample(rows[row]) is not None
        and v(selected_sample(rows[row]) or {}, "bucket30.resolutionCalled")
        and v(selected_sample(rows[row]) or {}, "bucket30.resolutionV22") == 1
        and not v(selected_sample(rows[row]) or {}, "bucket30.goodToCoolCommitted")
        for row in BASELINES
    ):
        print(
            "- 80014548 produced v22=1 but GOOD->COOL was not committed: "
            "target bucket30 finalization/writeback mapping."
        )
    if not any(rows.values()):
        print("- none of the PSX baseline rows appeared; capture likely missed gameplay/scorer window.")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("jsonl", type=Path, help="stage1_mem_probe JSONL capture")
    args = ap.parse_args()
    samples = load_jsonl(args.jsonl)
    print(f"# GOOD->COOL capture analysis: {args.jsonl}")
    print(f"samples: {len(samples)}")
    for row in BASELINES:
        print_row(samples, row)
    decision_hints(samples)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
