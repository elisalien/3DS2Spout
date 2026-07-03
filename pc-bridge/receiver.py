#!/usr/bin/env python3
"""3DS2SPOUT PC bridge (Python) — UDP QOI -> Spout + OSC relay."""
from __future__ import annotations

import ctypes
import socket
import struct
import sys
import time
from pathlib import Path

try:
    import numpy as np
    import qoi
except ImportError:
    print("Install: pip install qoi")
    sys.exit(1)

try:
    from pythonosc import udp_client
except ImportError:
    udp_client = None

PKT_MAGIC = 0xFC
PKT_HANDSHAKE = 0x00
PKT_FRAME = 0x01
PKT_CONTROL = 0x02

LISTEN_PORT = 5000
SPOUT_NAME = "3DS2SPOUT"
OSC_HOST = "127.0.0.1"
OSC_PORT = 7000


def get_lan_ipv4() -> str | None:
    """Best-effort LAN IPv4 for 3DS config (same trick as ip route)."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except OSError:
        return None


def print_pc_ip_banner(port: int = LISTEN_PORT) -> None:
    ip = get_lan_ipv4()
    print("")
    print("=" * 44)
    if ip:
        print(f"  IP PC pour la 3DS : {ip}")
        print(f"  Sur la 3DS : bouton IP PC -> saisir {ip}")
    else:
        print("  IP PC : (detectez avec ipconfig)")
        print("  Sur la 3DS : bouton IP PC -> saisie tactile")
    print(f"  Port UDP : {port}")
    print("=" * 44)
    print("")

OSC_MAP = {
    0x10: ("/composition/layers/1/video/effects/1/opacity", "float"),
    0x11: ("/composition/layers/1/video/opacity", "float"),
    0x20: ("/composition/layers/1/video/effects/1/enabled", "int"),
    0x21: ("/composition/layers/1/video/effects/2/enabled", "int"),
    0x22: ("/composition/layers/1/video/effects/3/enabled", "int"),
    0x23: ("/composition/layers/1/video/effects/4/enabled", "int"),
    0x24: ("/composition/layers/1/video/effects/5/enabled", "int"),
    0x25: ("/composition/layers/1/video/effects/6/enabled", "int"),
    0x30: ("/composition/layers/1/video/blendmode", "int"),
}

GL_RGBA = 0x1908


def find_spout_dll() -> Path | None:
    here = Path(__file__).resolve().parent
    for p in (
        here / "vendor" / "SpoutLibrary.dll",
        here / "SpoutLibrary.dll",
        Path("SpoutLibrary.dll"),
    ):
        if p.exists():
            return p
    return None


class SpoutSender:
    VTABLE_SET_SENDER_NAME = 0
    VTABLE_SEND_IMAGE = 5
    VTABLE_SET_CPU_SHARE = 137
    VTABLE_CREATE_OPENGL = 151

    def __init__(self, name: str) -> None:
        dll_path = find_spout_dll()
        if not dll_path:
            raise FileNotFoundError("SpoutLibrary.dll not found in pc-bridge/vendor/")
        print(f"SpoutLibrary: {dll_path}")
        self._lib = ctypes.CDLL(str(dll_path))
        self._lib.GetSpout.restype = ctypes.c_void_p
        self._obj = self._lib.GetSpout()
        if not self._obj:
            raise RuntimeError("GetSpout() failed")
        vtable = ctypes.cast(
            ctypes.cast(self._obj, ctypes.POINTER(ctypes.c_void_p))[0],
            ctypes.POINTER(ctypes.c_void_p),
        )
        self._set_name = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_char_p)(
            vtable[self.VTABLE_SET_SENDER_NAME]
        )
        self._send_image = ctypes.CFUNCTYPE(
            ctypes.c_bool,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_uint8),
            ctypes.c_uint,
            ctypes.c_uint,
            ctypes.c_uint,
            ctypes.c_bool,
        )(vtable[self.VTABLE_SEND_IMAGE])
        self._set_cpu_share = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_bool)(
            vtable[self.VTABLE_SET_CPU_SHARE]
        )
        self._create_opengl = ctypes.CFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)(
            vtable[self.VTABLE_CREATE_OPENGL]
        )
        if not self._create_opengl(self._obj, None):
            raise RuntimeError("CreateOpenGL() failed — mauvaise SpoutLibrary.dll (x64 MD requis)")
        self._set_cpu_share(self._obj, True)
        self._set_name(self._obj, name.encode("ascii"))
        self._buf: ctypes.Array[ctypes.c_uint8] | None = None

    def send_rgba(self, rgba: bytes, width: int, height: int) -> bool:
        needed = width * height * 4
        if self._buf is None or len(self._buf) != needed:
            self._buf = (ctypes.c_uint8 * needed)()
        ctypes.memmove(self._buf, rgba, needed)
        return bool(
            self._send_image(
                self._obj,
                self._buf,
                width,
                height,
                GL_RGBA,
                False,
            )
        )

    def send_test_pattern(self, width: int = 640, height: int = 480) -> bool:
        """Frame noire pour que Resolume voie la source avant le stream 3DS."""
        rgba = bytes([32, 32, 48, 255] * (width * height))
        return self.send_rgba(rgba, width, height)


def read_be32(b: bytes) -> int:
    return struct.unpack(">I", b)[0]


def read_be16(b: bytes) -> int:
    return struct.unpack(">H", b)[0]


def read_be_f32(b: bytes) -> float:
    return struct.unpack(">f", b)[0]


class FrameAssembler:
    """Réassemble les frames QOI chunkées.

    Chunk: FC 01 | seq be32 | total_size be32 | chunk_idx be16 |
    chunk_count be16 | payload_len be16 | payload (16-byte header).
    Une seule frame en cours : un seq plus récent jette l'incomplète.
    """

    HEADER = 16

    def __init__(self) -> None:
        self.seq = -1
        self.total = 0
        self.count = 0
        self.chunks: dict[int, bytes] = {}
        self.last_done = -1

    def feed(self, data: bytes) -> bytes | None:
        if len(data) < self.HEADER:
            return None
        seq = read_be32(data[2:6])
        total = read_be32(data[6:10])
        idx = read_be16(data[10:12])
        count = read_be16(data[12:14])
        plen = read_be16(data[14:16])
        if count == 0 or idx >= count or len(data) < self.HEADER + plen:
            return None
        if seq <= self.last_done:
            return None  # frame déjà livrée (chunk retardataire)
        if seq != self.seq:
            if seq < self.seq:
                return None  # chunk d'une vieille frame abandonnée
            self.seq = seq
            self.total = total
            self.count = count
            self.chunks = {}
        self.chunks[idx] = data[self.HEADER : self.HEADER + plen]
        if len(self.chunks) < self.count:
            return None
        frame = b"".join(self.chunks[i] for i in range(self.count))
        self.chunks = {}
        self.last_done = seq
        if len(frame) != self.total:
            return None
        return frame


def decode_qoi(data: bytes) -> tuple[bytes, int, int]:
    img = np.asarray(qoi.decode(data))
    h, w = img.shape[:2]
    if img.ndim == 3 and img.shape[2] == 3:
        alpha = np.full((h, w, 1), 255, dtype=img.dtype)
        img = np.concatenate([img, alpha], axis=2)
    return img.tobytes(), w, h


def main() -> None:
    print("3DS2SPOUT — PC Bridge (Python)")
    print(f"Listening UDP :{LISTEN_PORT}")
    print_pc_ip_banner(LISTEN_PORT)

    spout: SpoutSender | None = None
    try:
        spout = SpoutSender(SPOUT_NAME)
        print(f"Spout sender: {SPOUT_NAME}")
        if spout.send_test_pattern():
            print("Spout test frame OK — cherche '3DS2SPOUT' dans Sources Resolume")
        else:
            print("Spout: SendImage a echoue — verifie SpoutLibrary.dll x64 MD")
    except Exception as e:
        print(f"Spout warning: {e}")
        print("Video will decode but Resolume needs SpoutLibrary.dll in pc-bridge/vendor/")

    osc = None
    if udp_client:
        osc = udp_client.SimpleUDPClient(OSC_HOST, OSC_PORT)
        print(f"OSC relay -> {OSC_HOST}:{OSC_PORT}")
    else:
        print("OSC disabled (pip install python-osc)")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", LISTEN_PORT))
    sock.settimeout(0.05)

    assembler = FrameAssembler()
    fps_count = 0
    fps_t = time.time()
    last_dims = (0, 0)

    while True:
        try:
            data, addr = sock.recvfrom(65535)
        except socket.timeout:
            continue

        if len(data) < 2 or data[0] != PKT_MAGIC:
            continue

        ptype = data[1]
        if ptype == PKT_HANDSHAKE and len(data) >= 2:
            sock.sendto(bytes([PKT_MAGIC, PKT_HANDSHAKE, 1]), addr)
            print(f"Handshake from {addr[0]}")
        elif ptype == PKT_CONTROL and len(data) >= 7 and osc:
            pid = data[2]
            val = read_be_f32(data[3:7])
            if pid in OSC_MAP:
                path, typ = OSC_MAP[pid]
                if typ == "int":
                    osc.send_message(path, int(round(val)))
                else:
                    osc.send_message(path, float(val))
        elif ptype == PKT_FRAME:
            frame = assembler.feed(data)
            if frame is None:
                continue
            try:
                rgba, w, h = decode_qoi(frame)
            except Exception:
                continue
            if spout:
                spout.send_rgba(rgba, w, h)
            last_dims = (w, h)
            fps_count += 1
        if time.time() - fps_t >= 1.0:
            if fps_count:
                print(f"FPS: {fps_count} | {last_dims[0]}x{last_dims[1]}")
            fps_count = 0
            fps_t = time.time()


if __name__ == "__main__":
    main()
