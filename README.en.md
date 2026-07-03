# 3DS2SPOUT

[Français](README.md) · **English**

**The Nintendo 3DS camera as a Spout source — wirelessly.**

3DS2SPOUT turns a 3DS into a WiFi camera for VJing and live visuals: the image
is streamed to a Windows PC, which publishes it as a **Spout** source usable in
any compatible software (Resolume, TouchDesigner, OBS, MadMapper…). The console's
touch screen acts as a remote: camera switching, quality, and effects relayed
over **OSC**. FR/EN interface.

---

## How it works

```
3DS (camera + touch screen)  ──WiFi/UDP──▶  PC (bridge)  ──Spout──▶  your visual software
                                                         ──OSC───▶  effect control
```

No cables, no files to edit: the only configuration is the PC's IP address,
entered once on the 3DS touch screen.

---

## Installation

Step-by-step illustrated guide (French): [`install/guide/GUIDE-DEBUTANT.pdf`](install/guide/GUIDE-DEBUTANT.pdf)

Requirements: a Windows 10/11 PC with [Python 3.10+](https://www.python.org/downloads/)
and any Spout-compatible software, a 3DS with
[Luma3DS + Homebrew Launcher](https://3ds.hacks.guide), both on the same WiFi network.
Everything else (packages, Spout) installs automatically.

| # | Where | Step |
|---|-------|------|
| 1 | PC | Download the [release](../../releases) and run `install\INSTALL.cmd` |
| 2 | 3DS | Copy `3ds-cam-stream.3dsx` into the SD card's `3ds` folder (once) |
| 3 | PC | Run `install\run-bridge.cmd` — the IP to use is displayed |
| 4 | 3DS | Open **3DS2SPOUT** → **PC IP** button → enter the IP → **OK** |
| 5 | | In your visual software: Spout source **3DS2SPOUT**. On the 3DS: **REC** |

Details, options and troubleshooting: [`install/README.md`](install/README.md) (French)

---

## Controls (touch screen)

| Control | Action |
|---------|--------|
| **REC** | Start / stop the stream (`PC OK` + FPS when connected) |
| **OUTER / INNER** | Rear / front camera (shortcuts **Y** / **X**) |
| **SMOOTH 200P / HD 400P** | Stream quality |
| **PC IP** | PC IP address + FR/EN language |
| **START** | Quit |

---

## Repository layout

| Folder | Purpose |
|--------|---------|
| [`3ds-cam-stream/`](3ds-cam-stream/) | 3DS app — capture, touch UI, QOI/UDP stream |
| [`pc-bridge/`](pc-bridge/) | PC bridge: UDP → Spout + OSC relay (Python, or Rust) |
| [`resolume/`](resolume/) | Resolume integration example + OSC mapping |
| [`install/`](install/) | Install scripts and PDF guide |
| [`docs/`](docs/) | [Architecture & protocol](docs/ARCHITECTURE.md), [HOME menu icon](docs/FORWARDER-FBI.md) |

---

## For developers

Build the `.3dsx` with devkitPro (`pacman -S 3ds-dev`, then `make` in
`3ds-cam-stream/`), deploy over WiFi via FTPD (`scripts/install-to-3ds.ps1` or `.sh`),
package a release with `scripts/make-release.cmd`. The UDP protocol (handshake, QOI
frames, OSC control — port 5000) is described in
[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md), deployment in
[`scripts/README.md`](scripts/README.md).

---

## License

MIT — QOI encoder based on [phoboslab/qoi](https://github.com/phoboslab/qoi) (MIT).
