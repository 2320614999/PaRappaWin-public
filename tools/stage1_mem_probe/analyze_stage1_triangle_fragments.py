#!/usr/bin/env python3
import argparse
import csv
import importlib.util
import json
import math
from collections import Counter
from pathlib import Path


HANDLERS = {
    0: "Unsupported",
    1: "F3NL",
    2: "NF3",
    3: "TF3NL",
    4: "TNF3",
    5: "F4NL",
    6: "NF4",
    7: "TF4NL",
    8: "TNF4",
    9: "G3NL",
    10: "NG3",
    11: "TG3NL",
    12: "TNG3",
    13: "G4NL",
    14: "NG4",
    15: "TG4NL",
    16: "TNG4",
}


def parse_word(value):
    if isinstance(value, int):
        return value
    return int(str(value), 16)


def tri_area(xs, ys):
    if len(xs) < 3 or len(ys) < 3:
        return 0.0
    return abs((xs[1] - xs[0]) * (ys[2] - ys[0]) -
               (ys[1] - ys[0]) * (xs[2] - xs[0])) * 0.5


def bbox(xs, ys):
    if len(xs) < 3 or len(ys) < 3:
        return (0.0, 0.0)
    return (max(xs) - min(xs), max(ys) - min(ys))


def fmt_pts(xs, ys):
    return " ".join(f"({x:.3f},{y:.3f})" for x, y in zip(xs, ys))


def load_psx_gp0_keys(psxrec, frame):
    tool = Path(__file__).resolve().parents[1] / "模拟器内存回放器.py"
    if not tool.exists():
        tool = Path("tools") / "模拟器内存回放器.py"
    spec = importlib.util.spec_from_file_location("psxrec_reader", tool)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    rec = module.PSXRecording(str(psxrec))
    packets = rec.get_gp0_packets(frame)
    return {tuple(pkt.get("words", [])) for pkt in packets}


def load_trace(path):
    packets = {}
    triangles = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            rec = json.loads(line)
            kind = rec.get("kind")
            frame = int(rec.get("frame", 0))
            packet_ord = int(rec.get("packetOrdinalInFrame", -1))
            if kind == "packet":
                packets[(frame, packet_ord)] = rec
            elif kind == "triangle":
                triangles.append(rec)
    return packets, triangles


def enrich_triangle(rec, packet):
    packet_x = [float(v) for v in rec.get("packetX", [])[:3]]
    packet_y = [float(v) for v in rec.get("packetY", [])[:3]]
    draw_x = [float(v) for v in rec.get("x", [])[:3]]
    draw_y = [float(v) for v in rec.get("y", [])[:3]]
    screen_x = [float(v) for v in rec.get("screenX", [])[:3]]
    screen_y = [float(v) for v in rec.get("screenY", [])[:3]]

    packet_words = []
    gp0_words = []
    if packet is not None:
        packet_words = [parse_word(w) for w in packet.get("words", [])]
        if len(packet_words) > 1:
            gp0_words = packet_words[1:]

    packet_area = rec.get("packetArea")
    draw_area = rec.get("drawArea")
    screen_area = rec.get("screenArea")
    if packet_area is None:
        packet_area = tri_area(packet_x, packet_y)
    if draw_area is None:
        draw_area = tri_area(draw_x, draw_y)
    if screen_area is None:
        screen_area = tri_area(screen_x, screen_y)
    packet_w, packet_h = bbox(packet_x, packet_y)
    draw_w, draw_h = bbox(draw_x, draw_y)
    screen_w, screen_h = bbox(screen_x, screen_y)

    return {
        "frame": int(rec.get("frame", 0)),
        "packetOrdinalInFrame": int(rec.get("packetOrdinalInFrame", -1)),
        "triangleIndexInPacket": int(rec.get("triangleIndexInPacket", -1)),
        "descName": rec.get("descName", ""),
        "descAddr": rec.get("descAddr", ""),
        "modelSet": rec.get("modelSet", ""),
        "modelIndex": rec.get("modelIndex", ""),
        "sourceObjectIndex": rec.get("sourceObjectIndex", ""),
        "rawPrimitiveIndex": rec.get("rawPrimitiveIndex", ""),
        "rawPacketOffset": (packet or {}).get("rawPacketOffset", ""),
        "handler": rec.get("handler", ""),
        "handlerName": HANDLERS.get(int(rec.get("handler", -1)), "?"),
        "primitiveCode": (packet or {}).get("primitiveCode", ""),
        "primitiveFlag": (packet or {}).get("primitiveFlag", ""),
        "psxCallOrder": rec.get("psxCallOrder", ""),
        "textured": rec.get("textured", False),
        "semiTransparent": rec.get("semiTransparent", False),
        "tpage": rec.get("tpage", ""),
        "clut": rec.get("clut", ""),
        "packetArea": float(packet_area),
        "drawArea": float(draw_area),
        "screenArea": float(screen_area),
        "packetW": packet_w,
        "packetH": packet_h,
        "drawW": draw_w,
        "drawH": draw_h,
        "screenW": screen_w,
        "screenH": screen_h,
        "packetSXY": fmt_pts(packet_x, packet_y),
        "drawXY": fmt_pts(draw_x, draw_y),
        "screenXY": fmt_pts(screen_x, screen_y),
        "gp0Key": tuple(gp0_words),
    }


def write_csv(path, rows):
    fields = [
        "frame", "packetOrdinalInFrame", "triangleIndexInPacket",
        "descName", "descAddr", "modelSet", "modelIndex",
        "sourceObjectIndex", "rawPrimitiveIndex", "rawPacketOffset",
        "handler", "handlerName", "primitiveCode", "primitiveFlag",
        "psxCallOrder", "textured", "semiTransparent", "tpage", "clut",
        "packetArea", "drawArea", "screenArea",
        "packetW", "packetH", "drawW", "drawH", "screenW", "screenH",
        "psxExactPacket", "packetSXY", "drawXY", "screenXY",
    ]
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fields)
        writer.writeheader()
        for row in rows:
            out = {k: row.get(k, "") for k in fields}
            writer.writerow(out)


def main():
    ap = argparse.ArgumentParser(
        description="Find tiny or suspicious Stage1 TMD triangles from the external sidecar trace.")
    ap.add_argument("trace", help="stage1_triangle_trace_debugger JSONL output")
    ap.add_argument("--area-threshold", type=float, default=1.0,
                    help="PSX-space packet/draw area threshold")
    ap.add_argument("--screen-area-threshold", type=float, default=0.0,
                    help="Backbuffer-space area threshold; 0 disables it")
    ap.add_argument("--thin-min-dim", type=float, default=0.0,
                    help="Also flag triangles with min(packet bbox) <= this value")
    ap.add_argument("--thin-long-dim", type=float, default=8.0,
                    help="Long dimension required for --thin-min-dim")
    ap.add_argument("--desc", action="append", default=[],
                    help="Restrict to descName; can be passed multiple times")
    ap.add_argument("--limit", type=int, default=80)
    ap.add_argument("--csv", help="Write the candidate table to CSV")
    ap.add_argument("--all-csv", help="Write all triangle rows to CSV")
    ap.add_argument("--psxrec", help="Optional .psxrec for exact GP0 packet comparison")
    ap.add_argument("--psx-frame", type=int,
                    help="PSX recording frame used with --psxrec")
    ap.add_argument("--include-psx-missing", action="store_true",
                    help="Treat exact GP0 packet misses as candidates. Use only when the trace and psxrec frame are aligned.")
    args = ap.parse_args()

    packets, triangles = load_trace(args.trace)
    psx_keys = None
    if args.psxrec:
        if args.psx_frame is None:
            raise SystemExit("--psx-frame is required with --psxrec")
        psx_keys = load_psx_gp0_keys(Path(args.psxrec), args.psx_frame)

    desc_filter = {d.upper() for d in args.desc}
    rows = []
    for rec in triangles:
        if desc_filter and str(rec.get("descName", "")).upper() not in desc_filter:
            continue
        key = (int(rec.get("frame", 0)),
               int(rec.get("packetOrdinalInFrame", -1)))
        row = enrich_triangle(rec, packets.get(key))
        if psx_keys is not None:
            row["psxExactPacket"] = row["gp0Key"] in psx_keys
        else:
            row["psxExactPacket"] = ""
        rows.append(row)

    candidates = []
    for row in rows:
        tiny = (row["packetArea"] < args.area_threshold or
                row["drawArea"] < args.area_threshold)
        screen_tiny = (args.screen_area_threshold > 0.0 and
                       row["screenArea"] < args.screen_area_threshold)
        thin = False
        if args.thin_min_dim > 0.0:
            thin = (min(row["packetW"], row["packetH"]) <= args.thin_min_dim and
                    max(row["packetW"], row["packetH"]) >= args.thin_long_dim)
        psx_missing = (args.include_psx_missing and psx_keys is not None and
                       not row["psxExactPacket"])
        if tiny or screen_tiny or thin or psx_missing:
            candidates.append(row)

    candidates.sort(key=lambda r: (
        r["psxExactPacket"] is False,
        min(r["packetArea"], r["drawArea"]),
        -max(r["packetW"], r["packetH"]),
    ))

    print(f"triangles={len(rows)} packets={len(packets)} candidates={len(candidates)}")
    if psx_keys is not None:
        exact_count = sum(1 for row in rows if row["psxExactPacket"])
        print(f"psx_frame={args.psx_frame} psx_gp0_packets={len(psx_keys)} "
              f"exact_packet_matches={exact_count}")
    print("candidate_desc_counts=" +
          str(Counter(row["descName"] for row in candidates).most_common(20)))
    print("all_desc_counts=" +
          str(Counter(row["descName"] for row in rows).most_common(20)))

    for row in candidates[:args.limit]:
        psx = ""
        if psx_keys is not None:
            psx = f" psxExact={int(bool(row['psxExactPacket']))}"
        print(
            "frame={frame} pkt={packetOrdinalInFrame} tri={triangleIndexInPacket} "
            "desc={descName} model={modelSet}:{modelIndex}/{sourceObjectIndex} "
            "raw={rawPrimitiveIndex} handler={handlerName} code={primitiveCode} "
            "packetArea={packetArea:.6f} drawArea={drawArea:.6f} "
            "screenArea={screenArea:.3f} bbox={packetW:.3f}x{packetH:.3f}"
            "{psx} packetSXY={packetSXY}".format(**row, psx=psx)
        )

    if args.csv:
        write_csv(args.csv, candidates)
        print(f"wrote candidates csv: {args.csv}")
    if args.all_csv:
        write_csv(args.all_csv, rows)
        print(f"wrote all-triangles csv: {args.all_csv}")


if __name__ == "__main__":
    main()
