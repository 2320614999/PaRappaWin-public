#!/usr/bin/env python3
"""Extract a compact Stage1 gameplay timeline from stage1_full_flow_capture.log.

Reverse-engineering aid only.
This JSON is an offline inspection view and must not be used to drive runtime
Stage1 behavior in the Win port.
"""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter, defaultdict
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_LOG = REPO_ROOT / "stage1_full_flow_capture.log"
DEFAULT_OUTPUT = REPO_ROOT / "temp" / "ghidra_reports" / "stage1_full_flow_timeline_summary.json"

LINE_RE = re.compile(
    r"^\[(?P<kind>[A-Z]+)\]\s+t=(?P<time>\d+)ms\s+(?P<field>[A-Za-z0-9_]+)"
    r"(?:\s+addr=(?P<addr>0x[0-9A-Fa-f]+))?\s+(?P<before>.+?)\s+->\s+(?P<after>.+)$"
)
GATE_RE = re.compile(r"^\[GATE\]\s+t=(?P<time>\d+)ms\s+(?P<message>.+)$")

DEFAULT_FIELDS = [
    "event_cursor",
    "stream_done",
    "stream_id",
    "stream_flag",
    "hud_base_frame",
    "hud_desc_ptr",
    "hud2_id",
    "hud2_base_frame",
    "hud2_desc_ptr",
    "hud3_id",
    "hud3_base_frame",
    "hud3_desc_ptr",
    "ctx_text_ptr",
    "ctx_line_a_ptr",
    "ctx_line_b_ptr",
    "ctx_alt_line_cursor_a",
    "ctx_word42",
    "ctx_input_mask",
    "ctx_input_parsed",
    "ctx_res55",
    "ctx_res56",
    "ctx_res58",
    "ctx_res59",
    "ctx_res61",
    "ctx_res62",
    "ctx_res65",
]


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Summarize Stage1 capture log into grouped timeline entries.")
    parser.add_argument("--log", type=Path, default=DEFAULT_LOG)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--field",
        action="append",
        dest="fields",
        default=[],
        help="Field name to keep. If omitted, a curated Stage1 event/HUD set is used.",
    )
    parser.add_argument("--top", type=int, default=10, help="How many busiest fields to show in summary.")
    return parser


def _parse_log(path: Path, keep_fields: set[str]):
    grouped: dict[int, list[dict[str, object]]] = defaultdict(list)
    counts: Counter[str] = Counter()
    gates: list[dict[str, object]] = []

    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        gate_match = GATE_RE.match(line)
        if gate_match:
            gates.append(
                {
                    "time_ms": int(gate_match.group("time")),
                    "message": gate_match.group("message"),
                }
            )
            continue

        match = LINE_RE.match(line)
        if not match:
            continue

        field = match.group("field")
        if field not in keep_fields:
            continue

        time_ms = int(match.group("time"))
        entry = {
            "kind": match.group("kind"),
            "field": field,
            "before": match.group("before"),
            "after": match.group("after"),
        }
        if match.group("addr"):
            entry["addr"] = match.group("addr")
        grouped[time_ms].append(entry)
        counts[field] += 1

    timeline = [
        {
            "time_ms": time_ms,
            "changes": grouped[time_ms],
        }
        for time_ms in sorted(grouped)
    ]

    return {
        "analysis_only": True,
        "not_runtime_data_source": True,
        "gates": gates,
        "field_counts": [
            {"field": field, "count": count}
            for field, count in counts.most_common()
        ],
        "timeline": timeline,
    }


def main() -> int:
    parser = _build_parser()
    args = parser.parse_args()

    keep_fields = set(args.fields or DEFAULT_FIELDS)
    summary = _parse_log(args.log, keep_fields)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(f"log:      {args.log}")
    print(f"output:   {args.output}")
    print(f"fields:   {len(keep_fields)} tracked")
    print(f"gates:    {len(summary['gates'])}")
    print(f"timeline: {len(summary['timeline'])} grouped timestamps")
    for item in summary["field_counts"][: args.top]:
        print(f"  {item['field']}: {item['count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
