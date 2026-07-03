# Architecture

## Data flow

```
3DS CAMU capture (400×240 RGB565)
    → CPU light effects (optional)
    → preview top screen
    → RGB888 buffer
    → QOI encode
    → UDP → PC bridge
    → QOI decode → RGBA
    → Spout "3DS2SPOUT"
    → Resolume Arena layer

Touch UI changes
    → UDP control packets (0xFC 0x02)
    → PC bridge OSC relay
    → Resolume (port 7000)
```

## UDP protocol

All multi-byte integers are **big-endian** unless noted.

### Magic byte

First byte is always `0xFC`. Second byte is packet type.

### Handshake (`0xFC 0x00`)

**3DS → PC**

| Offset | Size | Field |
|--------|------|-------|
| 0 | 2 | `0xFC 0x00` |
| 2 | 4 | 3DS IP (network order) |
| 6 | 2 | Stream width |
| 8 | 2 | Stream height |

**PC → 3DS**

| Offset | Size | Field |
|--------|------|-------|
| 0 | 2 | `0xFC 0x00` |
| 2 | 1 | `1` = OK |

### Frame chunk (`0xFC 0x01`)

**3DS → PC** — une frame QOI est découpée en chunks de ≤1280 octets
(une frame 400×240 peut dépasser 64 Ko : impossible en un seul datagramme UDP,
et les petits paquets évitent la fragmentation IP sur le WiFi 3DS).

| Offset | Size | Field |
|--------|------|-------|
| 0 | 2 | `0xFC 0x01` |
| 2 | 4 | Sequence number (be32, même valeur pour tous les chunks d'une frame) |
| 6 | 4 | Taille totale QOI (be32) |
| 10 | 2 | Index du chunk (be16, 0-based) |
| 12 | 2 | Nombre de chunks (be16) |
| 14 | 2 | Taille payload de ce chunk (be16) |
| 16 | N | Payload QOI (≤1280 octets) |

Le PC réassemble par sequence number. Une frame incomplète est jetée dès
qu'un seq plus récent arrive ; les chunks retardataires sont ignorés.

### Control (`0xFC 0x02`)

**3DS → PC**

| Offset | Size | Field |
|--------|------|-------|
| 0 | 2 | `0xFC 0x02` |
| 2 | 1 | Parameter ID |
| 3 | 4 | Value (`f32` big-endian) |

PC maps `param_id` to OSC addresses via `osc-map.toml`.

## Parameter IDs

| ID | Name | Resolume target (default) |
|----|------|---------------------------|
| `0x10` | Effect intensity | Layer 1 effect 1 opacity |
| `0x11` | Layer opacity | Layer 1 video opacity |
| `0x20`–`0x25` | Preset 1–6 | Layer 1 effects 1–6 enabled |
| `0x30` | Blend mode | Layer 1 blend mode index |

## Threading (3DS)

| Thread | Priority | Work |
|--------|----------|------|
| Main | default | aptMainLoop, touch UI, preview blit |
| Camera | high | CAMU capture loop |
| Network | medium | QOI encode + UDP send |

Double-buffered frame queue between camera and network threads.

## Performance targets

- Capture: 400×240 @ ~20 FPS
- QOI frame size: ~80–200 KB typical
- WiFi: 802.11g sufficient for ~15–20 FPS
