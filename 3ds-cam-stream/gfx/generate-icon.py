#!/usr/bin/env python3
"""Generate 48x48 homebrew icon (PNG) for 3DS2SPOUT.

Design: inspired by the Spout logo (turquoise disc + coral ring on navy)
with kawaii eyes. Drawn on a 24x24 logical grid, scaled x2 -> 48x48.
Pure standard library (struct + zlib), no third-party dependency.
"""
from __future__ import annotations

import math
import struct
import zlib
from pathlib import Path

OUT = Path(__file__).resolve().parent / "icon.png"
G = 24
S = 2
SIZE = G * S

T      = (0, 0, 0, 0)
NAVY   = (36, 40, 96, 255)
NAVY_D = (26, 29, 72, 255)
RED    = (233, 76, 64, 255)
TEAL   = (60, 208, 200, 255)
TEAL_D = (42, 172, 166, 255)
EYE    = (22, 26, 60, 255)
WHITE  = (240, 250, 250, 255)
CHEEK  = (246, 150, 162, 255)


def build_grid():
    px = [[T] * G for _ in range(G)]

    def put(x, y, c):
        if 0 <= x < G and 0 <= y < G:
            px[y][x] = c

    def rrect(x0, y0, x1, y1, r):
        out = []
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                cx = min(max(x, x0 + r), x1 - r)
                cy = min(max(y, y0 + r), y1 - r)
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r + r:
                    out.append((x, y))
        return out

    for (x, y) in rrect(1, 1, 22, 22, 4):
        put(x, y, NAVY)
    for (x, y) in rrect(1, 14, 22, 22, 4):
        put(x, y, NAVY_D)

    cx = cy = 11.5
    for y in range(G):
        for x in range(G):
            if px[y][x] == T:
                continue
            d = math.hypot(x - cx, y - cy)
            if d <= 8.0:
                px[y][x] = TEAL_D if (y - cy) > 2.5 else TEAL
            elif 9.2 <= d <= 10.7:
                px[y][x] = RED

    for (ex, ey) in ((8, 9), (14, 9)):
        for dx in range(2):
            for dy in range(3):
                put(ex + dx, ey + dy, EYE)
        put(ex, ey, WHITE)
        put(ex + 1, ey + 2, EYE)

    for chx in (6, 17):
        put(chx, 13, CHEEK)
        put(chx, 14, CHEEK)

    put(9, 13, EYE)
    put(14, 13, EYE)
    for x in range(10, 14):
        put(x, 14, EYE)

    return px


def write_png(path):
    grid = build_grid()
    rows = []
    for y in range(G):
        line = b""
        for x in range(G):
            line += bytes(grid[y][x]) * S
        for _ in range(S):
            rows.append(b"\x00" + line)
    raw = b"".join(rows)
    compressed = zlib.compress(raw, 9)

    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    ihdr = struct.pack(">IIBBBBB", SIZE, SIZE, 8, 6, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", ihdr)
    png += chunk(b"IDAT", compressed)
    png += chunk(b"IEND", b"")
    path.write_bytes(png)


if __name__ == "__main__":
    write_png(OUT)
    print("wrote %s (%dx%d)" % (OUT, SIZE, SIZE))
