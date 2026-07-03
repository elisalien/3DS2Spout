#!/usr/bin/env python3
"""Generate 48x48 homebrew icon (PNG) for 3DS2SPOUT."""
from __future__ import annotations

import struct
import zlib
from pathlib import Path

OUT = Path(__file__).resolve().parent / "icon.png"
SIZE = 48

# Palette: dark blue-grey + turquoise (matches project branding)
BG = (26, 40, 48)
ACCENT = (74, 138, 148)
ACCENT2 = (106, 172, 182)
TEXT = (232, 238, 240)


def inside_round_rect(x: int, y: int, cx: int, cy: int, w: int, h: int, r: int) -> bool:
    if x < cx or x >= cx + w or y < cy or y >= cy + h:
        return False
    if x < cx + r and y < cy + r:
        return (x - (cx + r)) ** 2 + (y - (cy + r)) ** 2 <= r * r
    if x >= cx + w - r and y < cy + r:
        return (x - (cx + w - r - 1)) ** 2 + (y - (cy + r)) ** 2 <= r * r
    if x < cx + r and y >= cy + h - r:
        return (x - (cx + r)) ** 2 + (y - (cy + h - r - 1)) ** 2 <= r * r
    if x >= cx + w - r and y >= cy + h - r:
        return (x - (cx + w - r - 1)) ** 2 + (y - (cy + h - r - 1)) ** 2 <= r * r
    return True


def draw_lens(x: int, y: int) -> bool:
    cx, cy, r = 24, 22, 14
    return (x - cx) ** 2 + (y - cy) ** 2 <= r * r


def draw_spout_arrow(x: int, y: int) -> bool:
    # small arrow / stream to the right
    if 30 <= x <= 42 and 20 <= y <= 24:
        return True
    if 38 <= x <= 42 and 16 <= y <= 28 and x + y >= 52:
        return True
    return False


def pixel(x: int, y: int) -> tuple[int, int, int, int]:
    if not inside_round_rect(x, y, 2, 2, 44, 44, 10):
        return (0, 0, 0, 0)
    if draw_lens(x, y):
        ring = (x - 24) ** 2 + (y - 22) ** 2
        if ring >= 11 * 11:
            return (*ACCENT2, 255)
        return (*TEXT, 255)
    if draw_spout_arrow(x, y):
        return (*ACCENT, 255)
    # subtle grid dot
    if (x + y) % 9 == 0 and 8 < x < 40 and 8 < y < 40:
        return (ACCENT[0], ACCENT[1], ACCENT[2], 40)
    return (*BG, 255)


def write_png(path: Path) -> None:
    rows = []
    for y in range(SIZE):
        row = b"\x00"
        for x in range(SIZE):
            row += bytes(pixel(x, y))
        rows.append(row)
    raw = b"".join(rows)
    compressed = zlib.compress(raw, 9)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", SIZE, SIZE, 8, 6, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", ihdr)
    png += chunk(b"IDAT", compressed)
    png += chunk(b"IEND", b"")
    path.write_bytes(png)


if __name__ == "__main__":
    write_png(OUT)
    print(f"Icon written: {OUT}")
