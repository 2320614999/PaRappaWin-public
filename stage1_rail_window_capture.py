#!/usr/bin/env python3
"""Narrow Stage1 rail capture around the current paused DuckStation frame.

Capture goals:
- one-shot sample of the current paused frame
- short controlled forward window using repeated resume + interrupt bursts

This is a pure script/emulator-side capture. It reuses existing Stage1 rail
helpers and does not modify any Win runtime code paths.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import socket
import time
from pathlib import Path
from typing import Any

import stage1_dual_path_capture as dual
import stage1_rail_full_flow_capture as rail


REPO_ROOT = Path(__file__).resolve().parent
DEFAULT_OUTPUT_DIR = REPO_ROOT / "artifacts" / "stage1_rail_window_capture"


class ControlledGdb:
    def __init__(self, host: str, port: int):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))
        self.sock.settimeout(1.0)
        self.rxbuf = bytearray()
        try:
            self.sock.recv(64)
        except Exception:
            pass

    @staticmethod
    def _checksum(payload: str) -> int:
        return sum(payload.encode("ascii", errors="strict")) & 0xFF

    def _send_packet(self, payload: str) -> None:
        checksum = self._checksum(payload)
        packet = f"${payload}#{checksum:02x}".encode("ascii")
        self.sock.sendall(packet)

    @staticmethod
    def _is_stop_reply(payload: str) -> bool:
        if len(payload) < 3:
            return False
        if payload[0] not in ("S", "T", "W", "X"):
            return False
        return all(ch in "0123456789abcdefABCDEF" for ch in payload[1:3])

    def _recv_some(self) -> None:
        chunk = self.sock.recv(65536)
        if not chunk:
            raise RuntimeError("duckstation gdb connection closed")
        self.rxbuf.extend(chunk)

    def _try_parse_packet(self) -> str | None:
        while self.rxbuf and self.rxbuf[0] in (ord("+"), ord("-")):
            del self.rxbuf[0]

        dollar = self.rxbuf.find(b"$")
        if dollar < 0:
            return None
        if dollar > 0:
            del self.rxbuf[:dollar]

        hash_pos = self.rxbuf.find(b"#")
        if hash_pos < 0 or len(self.rxbuf) < hash_pos + 3:
            return None

        payload = bytes(self.rxbuf[1:hash_pos]).decode("ascii", errors="replace")
        del self.rxbuf[: hash_pos + 3]
        try:
            self.sock.sendall(b"+")
        except Exception:
            pass
        return payload

    def _read_reply(self, timeout_sec: float, *, allow_stop: bool) -> str:
        deadline = time.monotonic() + timeout_sec
        while True:
            payload = self._try_parse_packet()
            if payload is not None:
                if self._is_stop_reply(payload) and not allow_stop:
                    continue
                return payload
            remain = deadline - time.monotonic()
            if remain <= 0:
                raise TimeoutError("timed out waiting for gdb reply")
            self.sock.settimeout(max(0.05, remain))
            self._recv_some()

    def read_memory(self, address: int, length: int, retries: int = 3) -> bytes:
        last_error: Exception | None = None
        for _ in range(max(1, retries)):
            try:
                self._send_packet(f"m{address:08x},{length:x}")
                payload = self._read_reply(2.5, allow_stop=False)
                if payload.startswith("E"):
                    raise RuntimeError(f"gdb memory read failed: {payload}")
                return bytes.fromhex(payload.strip())
            except Exception as exc:
                last_error = exc
                time.sleep(0.02)
        raise RuntimeError(f"failed to read 0x{address:08X}+0x{length:X}: {last_error}")

    def continue_run(self) -> None:
        self._send_packet("c")

    def interrupt(self) -> None:
        self.sock.sendall(b"\x03")

    def wait_stop(self, timeout_sec: float = 2.0) -> str:
        return self._read_reply(timeout_sec, allow_stop=True)

    def close(self) -> None:
        try:
            self.sock.close()
        except Exception:
            pass


def _write_json(path: Path, payload: Any) -> None:
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")


def _write_jsonl(path: Path, payload: Any) -> None:
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(payload, ensure_ascii=False) + "\n")


def _stream_signature(stream: dict[str, Any] | None) -> tuple[Any, ...]:
    if not stream:
        return (None, None, ())
    return (
        stream.get("head0"),
        stream.get("head1"),
        tuple(stream.get("stream") or ()),
    )


def _voice_signature(record: dict[str, Any]) -> tuple[int, int, int, int, int]:
    voice_lane = record["rail_sfx"]["voice_lane"]
    return rail._voice_signature(voice_lane)


def _stream18(stream: dict[str, Any] | None) -> list[int] | None:
    if not stream:
        return None
    values = stream.get("stream18")
    if values is None:
        return None
    return list(values)


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


def _capture_frame(
    reader: ControlledGdb,
    debug_client: rail.DebugClient,
    overlay: dict[str, Any],
    portrait_anim: dict[str, dict[str, Any]],
    prev_record: dict[str, Any] | None,
    sample_index: int,
    elapsed_ms: int,
) -> tuple[dict[str, Any], dict[str, Any]]:
    base_snapshot = dual.read_snapshot(reader, overlay)
    raw = rail._read_stage1_rail_raw(reader)
    event_cursor = int(base_snapshot["overlay"]["main_event_cursor"])
    main_event_current = rail._read_main_event(reader, event_cursor if event_cursor >= 0 else -1)
    main_event_next = rail._read_main_event(reader, event_cursor + 1)
    record = rail._build_record(
        sample_index=sample_index,
        elapsed_ms=elapsed_ms,
        base_snapshot=base_snapshot,
        raw=raw,
        main_event_current=main_event_current,
        main_event_next=main_event_next,
        portrait_anim=portrait_anim,
        prev_record=prev_record,
    )
    win_sample = rail.query_win_stage1_rail(debug_client)
    win_timeline = win_sample["timeline"]
    comparison = rail.build_dual_comparison(record, win_timeline)

    current_frame = {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_window_current_frame",
        "sample_index": sample_index,
        "elapsed_ms": elapsed_ms,
        "timing": record["timing"],
        "row_mode": raw.get("row_mode"),
        "teacher": {
            "cursor": raw.get("teacher_cursor"),
            "aux": raw.get("teacher_aux_cursor"),
            "enable": raw.get("teacher_enable"),
            "portrait": record["rail_display"]["teacher_portrait"],
            "stream94": raw.get("stream94"),
            "stream98": raw.get("stream98"),
            "stream94_stream18": _stream18(raw.get("stream94")),
            "stream98_stream18": _stream18(raw.get("stream98")),
        },
        "student": {
            "cursor": raw.get("student_cursor"),
            "aux": raw.get("student_aux_cursor"),
            "enable": raw.get("student_enable"),
            "portrait": record["rail_display"]["student_portrait"],
            "streamA4": raw.get("streamA4"),
            "streamA8": raw.get("streamA8"),
            "streamA4_stream18": _stream18(raw.get("streamA4")),
            "streamA8_stream18": _stream18(raw.get("streamA8")),
        },
        "icon_stream18": _stream18(raw.get("stream94")),
        "body_stream18": _stream18(raw.get("streamA4")),
        "overlay": base_snapshot["overlay"],
        "main_event_current": main_event_current,
        "main_event_next": main_event_next,
        "late_branch": base_snapshot["late_branch"],
        "accepted_formula": base_snapshot["accepted_formula"],
        "rail_display": {
            "visible": record["rail_display"]["visible"],
            "display_mode": record["rail_display"]["display_mode"],
            "icon_codes": record["rail_display"]["icon_codes"],
            "teacher_portrait": record["rail_display"]["teacher_portrait"],
            "student_portrait": record["rail_display"]["student_portrait"],
        },
        "rail_sfx": record["rail_sfx"],
        "win_rail": win_timeline,
        "comparison": comparison,
    }
    return current_frame, record


def _change_tags(prev_frame: dict[str, Any] | None, prev_record: dict[str, Any] | None, frame: dict[str, Any], record: dict[str, Any]) -> list[str]:
    tags: list[str] = []
    if prev_frame is None or prev_record is None:
        return ["capture_start"]

    prev_display = prev_frame["rail_display"]
    curr_display = frame["rail_display"]

    if prev_display["visible"] != curr_display["visible"]:
        tags.append("rail_visibility_change")
    if prev_display["display_mode"] != curr_display["display_mode"]:
        tags.append("display_mode_change")
    if prev_display["teacher_portrait"]["visible"] != curr_display["teacher_portrait"]["visible"]:
        tags.append("teacher_visibility_change")
        if curr_display["teacher_portrait"]["visible"]:
            tags.append("teacher_appears")
    if prev_display["student_portrait"]["visible"] != curr_display["student_portrait"]["visible"]:
        tags.append("student_visibility_change")
        if curr_display["student_portrait"]["visible"]:
            tags.append("student_appears")
    if prev_frame["teacher"]["cursor"] != frame["teacher"]["cursor"]:
        tags.append("teacher_cursor_change")
    if prev_frame["teacher"]["aux"] != frame["teacher"]["aux"]:
        tags.append("teacher_aux_change")
    if prev_frame["student"]["cursor"] != frame["student"]["cursor"]:
        tags.append("student_cursor_change")
    if prev_frame["student"]["aux"] != frame["student"]["aux"]:
        tags.append("student_aux_change")
    if prev_frame["icon_stream18"] != frame["icon_stream18"]:
        tags.append("icon_stream18_change")
    if _stream_signature(prev_frame["student"]["streamA4"]) != _stream_signature(frame["student"]["streamA4"]):
        tags.append("body_streamA4_change")
    if _stream_signature(prev_frame["student"]["streamA8"]) != _stream_signature(frame["student"]["streamA8"]):
        tags.append("body_streamA8_change")
    if prev_frame["overlay"]["resolved_event"] != frame["overlay"]["resolved_event"]:
        tags.append("resolved_event_change")
    if prev_frame["late_branch"] != frame["late_branch"]:
        tags.append("late_branch_change")
    if prev_frame["accepted_formula"] != frame["accepted_formula"]:
        tags.append("accepted_formula_change")
    if _voice_signature(prev_record) != _voice_signature(record):
        tags.append("rail_sfx_change")
    prev_win = prev_frame.get("win_rail") or {}
    curr_win = frame.get("win_rail") or {}
    if prev_win.get("parse_ok") != curr_win.get("parse_ok"):
        tags.append("win_parse_state_change")
    if _cue_signature((prev_win.get("rail") or {}).get("currentCommittedCue")) != _cue_signature(
        (curr_win.get("rail") or {}).get("currentCommittedCue")
    ):
        tags.append("win_current_cue_change")
    if (
        _cue_signature((prev_win.get("rail") or {}).get("nextLookaheadCue")),
        (prev_win.get("rail") or {}).get("nextLookaheadCueWindowActive"),
        (prev_win.get("rail") or {}).get("nextLookaheadAcceptedContributionEnabled"),
    ) != (
        _cue_signature((curr_win.get("rail") or {}).get("nextLookaheadCue")),
        (curr_win.get("rail") or {}).get("nextLookaheadCueWindowActive"),
        (curr_win.get("rail") or {}).get("nextLookaheadAcceptedContributionEnabled"),
    ):
        tags.append("win_next_cue_change")
    if (
        _cue_signature((prev_win.get("rail") or {}).get("acceptedBoundaryCue")),
        (prev_win.get("rail") or {}).get("acceptedBoundarySourceCellGateActive"),
        (prev_win.get("rail") or {}).get("acceptedBoundaryMaterialized"),
        ((prev_win.get("rail") or {}).get("acceptedHandoffPhase") or {}).get("value"),
    ) != (
        _cue_signature((curr_win.get("rail") or {}).get("acceptedBoundaryCue")),
        (curr_win.get("rail") or {}).get("acceptedBoundarySourceCellGateActive"),
        (curr_win.get("rail") or {}).get("acceptedBoundaryMaterialized"),
        ((curr_win.get("rail") or {}).get("acceptedHandoffPhase") or {}).get("value"),
    ):
        tags.append("win_accepted_cue_change")
    if tuple((prev_win.get("rail") or {}).get("bodyStreamA") or ()) != tuple(
        (curr_win.get("rail") or {}).get("bodyStreamA") or ()
    ):
        tags.append("win_body_stream_change")
    if _actor_signature((prev_win.get("cursor") or {}).get("teacher")) != _actor_signature(
        (curr_win.get("cursor") or {}).get("teacher")
    ):
        tags.append("win_teacher_handoff_change")
    if _actor_signature((prev_win.get("cursor") or {}).get("student")) != _actor_signature(
        (curr_win.get("cursor") or {}).get("student")
    ):
        tags.append("win_student_handoff_change")
    if (
        (prev_win.get("displayGate") or {}).get("lessonSelector"),
        (prev_win.get("displayGate") or {}).get("lessonVisible"),
        (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("narrowVisibleCandidate")),
        (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("cueWindowGateActive")),
        (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("sourceCellGateActive")),
        (((prev_win.get("displayGate") or {}).get("railVisibility") or {}).get("materializedGateActive")),
    ) != (
        (curr_win.get("displayGate") or {}).get("lessonSelector"),
        (curr_win.get("displayGate") or {}).get("lessonVisible"),
        (((curr_win.get("displayGate") or {}).get("railVisibility") or {}).get("narrowVisibleCandidate")),
        (((curr_win.get("displayGate") or {}).get("railVisibility") or {}).get("cueWindowGateActive")),
        (((curr_win.get("displayGate") or {}).get("railVisibility") or {}).get("sourceCellGateActive")),
        (((curr_win.get("displayGate") or {}).get("railVisibility") or {}).get("materializedGateActive")),
    ):
        tags.append("win_display_gate_change")

    for reason in record["reasons"]:
        if reason not in tags:
            tags.append(reason)
    return tags


def _window_entry(
    step_index: int,
    phase: str,
    burst_ms: int,
    stop_reply: str | None,
    frame: dict[str, Any],
    record: dict[str, Any],
    change_tags: list[str],
) -> dict[str, Any]:
    return {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_window_step",
        "phase": phase,
        "step_index": step_index,
        "burst_ms": burst_ms,
        "elapsed_ms": frame["elapsed_ms"],
        "stop_reply": stop_reply,
        "change_tags": change_tags,
        "timing": frame["timing"],
        "focus": {
            "rail_visible": frame["rail_display"]["visible"],
            "display_mode": frame["rail_display"]["display_mode"],
            "teacher_visible": frame["teacher"]["portrait"]["visible"],
            "teacher_cursor": frame["teacher"]["cursor"],
            "teacher_aux": frame["teacher"]["aux"],
            "teacher_enable": frame["teacher"]["enable"],
            "student_visible": frame["student"]["portrait"]["visible"],
            "student_cursor": frame["student"]["cursor"],
            "student_aux": frame["student"]["aux"],
            "student_enable": frame["student"]["enable"],
            "icon_stream18": frame["icon_stream18"],
            "body_stream18": frame["body_stream18"],
            "resolved_event": frame["overlay"]["resolved_event"],
            "win_queryFrame": ((frame.get("win_rail") or {}).get("rail") or {}).get("queryFrame"),
            "win_lessonSelector": ((frame.get("win_rail") or {}).get("displayGate") or {}).get("lessonSelector"),
            "win_lessonVisible": ((frame.get("win_rail") or {}).get("displayGate") or {}).get("lessonVisible"),
            "win_narrowVisibleCandidate": ((((frame.get("win_rail") or {}).get("displayGate") or {}).get("railVisibility") or {}).get("narrowVisibleCandidate")),
            "win_currentCommittedCue": ((frame.get("win_rail") or {}).get("rail") or {}).get("currentCommittedCue"),
            "win_nextLookaheadCue": ((frame.get("win_rail") or {}).get("rail") or {}).get("nextLookaheadCue"),
            "win_acceptedBoundaryCue": ((frame.get("win_rail") or {}).get("rail") or {}).get("acceptedBoundaryCue"),
        },
        "rail_sfx": record["rail_sfx"] if ("rail_sfx_change" in change_tags or step_index == 0) else None,
        "win_rail": frame.get("win_rail"),
        "comparison": frame.get("comparison"),
    }


def _interesting_events(entries: list[dict[str, Any]]) -> dict[str, list[dict[str, Any]]]:
    buckets = {
        "rail_visibility_changes": [],
        "teacher_appears": [],
        "student_appears": [],
        "body_stream_changes": [],
        "icon_stream_changes": [],
        "rail_sfx_changes": [],
        "win_visibility_changes": [],
        "win_cue_changes": [],
    }
    for entry in entries:
        tags = set(entry["change_tags"])
        marker = {
            "step_index": entry["step_index"],
            "elapsed_ms": entry["elapsed_ms"],
            "tick96": entry["timing"]["tick96"],
            "bucket": entry["timing"]["bucket"],
            "display_mode": entry["focus"]["display_mode"],
            "resolved_event_frame": (entry["focus"]["resolved_event"] or {}).get("frame"),
        }
        if "rail_visibility_change" in tags:
            buckets["rail_visibility_changes"].append(marker)
        if "teacher_appears" in tags:
            buckets["teacher_appears"].append(marker)
        if "student_appears" in tags:
            buckets["student_appears"].append(marker)
        if "body_streamA4_change" in tags or "body_streamA8_change" in tags:
            buckets["body_stream_changes"].append(marker)
        if "icon_stream18_change" in tags:
            buckets["icon_stream_changes"].append(marker)
        if "rail_sfx_change" in tags:
            buckets["rail_sfx_changes"].append(marker)
        if "win_display_gate_change" in tags:
            buckets["win_visibility_changes"].append(marker)
        if (
            "win_current_cue_change" in tags or
            "win_next_cue_change" in tags or
            "win_accepted_cue_change" in tags
        ):
            buckets["win_cue_changes"].append(marker)
    return buckets


def capture(args: argparse.Namespace) -> int:
    overlay = dual.load_overlay_model(args.comod, args.compo)
    output_dir = rail._unique_output_dir(args.output_dir, args.label)
    current_path = output_dir / "current_frame.json"
    window_path = output_dir / "window.jsonl"
    summary_path = output_dir / "summary.json"
    session_path = output_dir / "session.json"

    reader = ControlledGdb(args.host, args.port)
    debug = rail.DebugClient(host=args.debug_host, port=args.debug_port, timeout=args.debug_timeout_sec)
    portrait_anim = {"teacher": {}, "student": {}}
    start_time = time.monotonic()
    initial_frame: dict[str, Any] | None = None
    prev_frame: dict[str, Any] | None = None
    prev_record: dict[str, Any] | None = None
    entries: list[dict[str, Any]] = []
    final_pause_reply: str | None = None

    try:
        frame0, record0 = _capture_frame(reader, debug, overlay, portrait_anim, None, 0, 0)
        initial_frame = frame0
        _write_json(current_path, frame0)
        first_entry = _window_entry(
            step_index=0,
            phase="current_paused",
            burst_ms=0,
            stop_reply=None,
            frame=frame0,
            record=record0,
            change_tags=["capture_start"],
        )
        entries.append(first_entry)
        _write_jsonl(window_path, first_entry)
        prev_frame = frame0
        prev_record = record0

        for step_index in range(1, args.steps + 1):
            reader.continue_run()
            time.sleep(args.burst_ms / 1000.0)
            reader.interrupt()
            stop_reply = reader.wait_stop(args.stop_timeout_sec)
            elapsed_ms = int((time.monotonic() - start_time) * 1000.0)
            frame, record = _capture_frame(
                reader,
                debug,
                overlay,
                portrait_anim,
                prev_record,
                step_index,
                elapsed_ms,
            )
            tags = _change_tags(prev_frame, prev_record, frame, record)
            entry = _window_entry(
                step_index=step_index,
                phase="short_resume",
                burst_ms=args.burst_ms,
                stop_reply=stop_reply,
                frame=frame,
                record=record,
                change_tags=tags,
            )
            entries.append(entry)
            _write_jsonl(window_path, entry)
            prev_frame = frame
            prev_record = record
    finally:
        try:
            reader.interrupt()
            final_pause_reply = reader.wait_stop(1.0)
        except Exception:
            final_pause_reply = final_pause_reply or "pause_confirmation_unavailable"
        reader.close()

    interesting = _interesting_events(entries)
    current_frame = initial_frame if initial_frame is not None else {}
    summary = {
        "api": "openai-responses",
        "capture_kind": "stage1_rail_window_capture",
        "captured_at": _dt.datetime.now().isoformat(),
        "host": args.host,
        "port": args.port,
        "steps_requested": args.steps,
        "steps_captured": len(entries) - 1,
        "burst_ms": args.burst_ms,
        "backward_sampling": "not attempted; no rewind path in this narrow GDB workflow",
        "final_pause_reply": final_pause_reply,
        "files": {
            "current_frame": str(current_path),
            "window": str(window_path),
            "summary": str(summary_path),
        },
        "current_confirmation": {
            "rail_visible": current_frame.get("rail_display", {}).get("visible"),
            "display_mode": current_frame.get("rail_display", {}).get("display_mode"),
            "teacher_visible": current_frame.get("teacher", {}).get("portrait", {}).get("visible"),
            "student_visible": current_frame.get("student", {}).get("portrait", {}).get("visible"),
            "icon_stream_has_nonzero": any((current_frame.get("icon_stream18") or [])),
            "body_stream_has_nonzero": any((current_frame.get("body_stream18") or [])),
            "resolved_event_frame": (current_frame.get("overlay", {}).get("resolved_event") or {}).get("frame"),
            "win_queryFrame": ((current_frame.get("win_rail") or {}).get("rail") or {}).get("queryFrame"),
            "win_lessonSelector": ((current_frame.get("win_rail") or {}).get("displayGate") or {}).get("lessonSelector"),
            "win_narrowVisibleCandidate": ((((current_frame.get("win_rail") or {}).get("displayGate") or {}).get("railVisibility") or {}).get("narrowVisibleCandidate")),
        },
        "interesting_events": interesting,
    }
    _write_json(summary_path, summary)
    _write_json(
        session_path,
        {
            "api": "openai-responses",
            "script": str(Path(__file__).resolve()),
            "captured_at": _dt.datetime.now().isoformat(),
            "args": {
                "host": args.host,
                "port": args.port,
                "debug_host": args.debug_host,
                "debug_port": args.debug_port,
                "steps": args.steps,
                "burst_ms": args.burst_ms,
                "stop_timeout_sec": args.stop_timeout_sec,
                "label": args.label,
                "output_dir": str(args.output_dir),
            },
            "files": {
                "current_frame": str(current_path),
                "window": str(window_path),
                "summary": str(summary_path),
                "session": str(session_path),
            },
            "final_pause_reply": final_pause_reply,
        },
    )
    print(f"[OK] wrote {output_dir}")
    return 0


def build_argparser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Narrow Stage1 rail capture around current paused DuckStation state")
    parser.add_argument("--host", default=dual.GDB_HOST)
    parser.add_argument("--port", type=int, default=dual.GDB_PORT)
    parser.add_argument("--debug-host", default=rail.DEBUG_HOST)
    parser.add_argument("--debug-port", type=int, default=rail.DEBUG_PORT)
    parser.add_argument("--debug-timeout-sec", type=float, default=2.0)
    parser.add_argument("--steps", type=int, default=10)
    parser.add_argument("--burst-ms", type=int, default=30)
    parser.add_argument("--stop-timeout-sec", type=float, default=2.0)
    parser.add_argument("--label", default="current_paused_rail_window")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--comod", type=Path, default=dual.DEFAULT_COMOD)
    parser.add_argument("--compo", type=Path, default=dual.DEFAULT_COMPO)
    return parser


def main() -> int:
    parser = build_argparser()
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    return capture(args)


if __name__ == "__main__":
    raise SystemExit(main())
