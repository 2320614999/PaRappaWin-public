#!/usr/bin/env python3
import argparse
import json
import math
from collections import defaultdict


EDGE_VERTS = ((0, 1), (1, 2), (2, 0))


def point_key(x, y):
    return (int(round(x)), int(round(y)))


def canonical_edge_key(a, b):
    return tuple(sorted((a, b)))


def dist(a, b):
    return math.hypot(a[0] - b[0], a[1] - b[1])


def edge_normal(edge):
    (x0, y0), (x1, y1) = edge
    dx = x1 - x0
    dy = y1 - y0
    length = math.hypot(dx, dy)
    if length <= 1.0e-6:
        return (0.0, 0.0)
    return (-dy / length, dx / length)


def edge_gap(edge_a, edge_b):
    d0 = dist(edge_a[0], edge_b[0])
    d1 = dist(edge_a[1], edge_b[1])
    mean_edge = (
        ((edge_a[0][0] + edge_b[0][0]) * 0.5,
         (edge_a[0][1] + edge_b[0][1]) * 0.5),
        ((edge_a[1][0] + edge_b[1][0]) * 0.5,
         (edge_a[1][1] + edge_b[1][1]) * 0.5),
    )
    nx, ny = edge_normal(mean_edge)
    n0 = abs((edge_a[0][0] - edge_b[0][0]) * nx +
             (edge_a[0][1] - edge_b[0][1]) * ny)
    n1 = abs((edge_a[1][0] - edge_b[1][0]) * nx +
             (edge_a[1][1] - edge_b[1][1]) * ny)
    return {
        "endpointMax": max(d0, d1),
        "endpointMean": (d0 + d1) * 0.5,
        "normalMax": max(n0, n1),
        "normalMean": (n0 + n1) * 0.5,
    }


def edge_identity(rec, edge_index, a, b):
    slots = rec.get("slots") or [0, 1, 2]
    slot_pair = tuple(sorted((int(slots[a]), int(slots[b]))))
    return (
        int(rec.get("rawPrimitiveIndex", -1)),
        int(rec.get("triangleIndexInPacket", -1)),
        int(edge_index),
        slot_pair,
    )


def make_edge(rec, edge_index):
    a, b = EDGE_VERTS[edge_index]
    px = rec.get("packetX") or []
    py = rec.get("packetY") or []
    sx = rec.get("screenX") or []
    sy = rec.get("screenY") or []
    if len(px) < 3 or len(py) < 3 or len(sx) < 3 or len(sy) < 3:
        return None
    packet_a = point_key(px[a], py[a])
    packet_b = point_key(px[b], py[b])
    key = canonical_edge_key(packet_a, packet_b)
    screen_a = (float(sx[a]), float(sy[a]))
    screen_b = (float(sx[b]), float(sy[b]))
    if (packet_a, packet_b) == key:
        screen_edge = (screen_a, screen_b)
    else:
        screen_edge = (screen_b, screen_a)
    return {
        "key": key,
        "screen": screen_edge,
        "identity": edge_identity(rec, edge_index, a, b),
        "rec": rec,
    }


def load_triangles(path, include_textured, include_semi):
    frames = defaultdict(list)
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            rec = json.loads(line)
            if rec.get("kind") != "triangle":
                continue
            if not rec.get("packetGeometryKnown", False):
                continue
            if not include_textured and rec.get("textured", False):
                continue
            if not include_semi and rec.get("semiTransparent", False):
                continue
            frames[int(rec.get("frame", 0))].append(rec)
    return frames


def collect_pairs(frames, exclude_same_primitive):
    by_pair = defaultdict(list)
    rows = []
    for frame, tris in frames.items():
        edges_by_key = defaultdict(list)
        for rec in tris:
            for edge_index in range(3):
                edge = make_edge(rec, edge_index)
                if edge is not None:
                    edges_by_key[edge["key"]].append(edge)
        for packet_key, edges in edges_by_key.items():
            if len(edges) < 2:
                continue
            for i in range(len(edges)):
                for j in range(i + 1, len(edges)):
                    a = edges[i]
                    b = edges[j]
                    rec_a = a["rec"]
                    rec_b = b["rec"]
                    same_primitive = (
                        rec_a.get("rawPrimitiveIndex") ==
                        rec_b.get("rawPrimitiveIndex")
                    )
                    if exclude_same_primitive and same_primitive:
                        continue
                    gap = edge_gap(a["screen"], b["screen"])
                    pair_id = tuple(sorted((a["identity"], b["identity"])))
                    row = {
                        "frame": frame,
                        "packetEdge": packet_key,
                        "pairId": pair_id,
                        "edgeA": a,
                        "edgeB": b,
                        "samePrimitive": same_primitive,
                        "gap": gap,
                    }
                    by_pair[pair_id].append(row)
                    rows.append(row)
    return rows, by_pair


def add_temporal_scores(by_pair, max_frame_gap):
    for series in by_pair.values():
        series.sort(key=lambda r: r["frame"])
        prev = None
        for cur in series:
            cur["temporalBetter"] = False
            cur["crossPrevMin"] = None
            if (prev is not None and
                    0 < cur["frame"] - prev["frame"] <= max_frame_gap):
                same = cur["gap"]["endpointMax"]
                cross_ab = edge_gap(cur["edgeA"]["screen"],
                                    prev["edgeB"]["screen"])["endpointMax"]
                cross_ba = edge_gap(prev["edgeA"]["screen"],
                                    cur["edgeB"]["screen"])["endpointMax"]
                cross_min = min(cross_ab, cross_ba)
                cur["crossPrevMin"] = cross_min
                cur["temporalBetter"] = same > 1.0e-6 and cross_min < same * 0.5
            prev = cur


def summarize(rows, limit):
    ranked = sorted(
        rows,
        key=lambda r: (
            r["gap"]["normalMax"],
            r["gap"]["endpointMax"],
            1 if r.get("temporalBetter") else 0,
        ),
        reverse=True,
    )
    for r in ranked[:limit]:
        a = r["edgeA"]["rec"]
        b = r["edgeB"]["rec"]
        print(
            "frame={frame} normalMax={normal:.3f} endpointMax={endpoint:.3f} "
            "temporalBetter={temporal} crossPrevMin={cross} "
            "samePrim={same} edge={edge} "
            "A(raw={raw_a},tri={tri_a},packetOrd={pkt_a},order={order_a}) "
            "B(raw={raw_b},tri={tri_b},packetOrd={pkt_b},order={order_b})".format(
                frame=r["frame"],
                normal=r["gap"]["normalMax"],
                endpoint=r["gap"]["endpointMax"],
                temporal=r.get("temporalBetter", False),
                cross=(
                    "None" if r.get("crossPrevMin") is None
                    else f"{r['crossPrevMin']:.3f}"
                ),
                same=r["samePrimitive"],
                edge=r["packetEdge"],
                raw_a=a.get("rawPrimitiveIndex"),
                tri_a=a.get("triangleIndexInPacket"),
                pkt_a=a.get("packetOrdinalInFrame"),
                order_a=a.get("order"),
                raw_b=b.get("rawPrimitiveIndex"),
                tri_b=b.get("triangleIndexInPacket"),
                pkt_b=b.get("packetOrdinalInFrame"),
                order_b=b.get("order"),
            )
        )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("trace")
    ap.add_argument("--include-textured", action="store_true")
    ap.add_argument("--include-semi", action="store_true")
    ap.add_argument("--exclude-same-primitive", action="store_true")
    ap.add_argument("--temporal-frame-gap", type=int, default=2)
    ap.add_argument("--limit", type=int, default=40)
    args = ap.parse_args()

    frames = load_triangles(args.trace, args.include_textured, args.include_semi)
    rows, by_pair = collect_pairs(frames, args.exclude_same_primitive)
    add_temporal_scores(by_pair, args.temporal_frame_gap)
    print(f"frames={len(frames)} shared_edge_pairs={len(rows)}")
    temporal_hits = sum(1 for r in rows if r.get("temporalBetter"))
    print(f"temporal_better_pairs={temporal_hits}")
    summarize(rows, args.limit)


if __name__ == "__main__":
    main()
