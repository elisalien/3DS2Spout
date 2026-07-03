# 3DS2SPOUT — Resolume Arena Setup

## Prérequis

| Élément | Détail |
|---------|--------|
| **PC bridge lancé** | `install\run-bridge.cmd` |
| **SpoutLibrary.dll** | Dans `pc-bridge/vendor/` |
| **IP 3DS configurée** | Bouton **IP PC** sur la console (IP affichée par le bridge) |
| **Resolume Arena** | Installé sur le même PC que le bridge |

---

## Ordre exact

```
1. PC  → install\run-bridge.cmd (noter l'IP PC)
2. 3DS → 3DS2SPOUT → IP PC → saisir l'IP → OK
3. PC  → Resolume Arena (étapes ci-dessous)
4. 3DS → REC
```

---

## 1. Activer OSC (optionnel)

1. **Resolume Arena** → **Preferences** → **OSC**
2. Activer **OSC Input** (port **7000** par défaut)
3. Vérifier `osc_host` / `osc_port` dans [`../pc-bridge/config.toml`](../pc-bridge/config.toml)

---

## 2. Bridge PC

```cmd
cd /d D:\CREA\TOOLS\3DS\apps
install\run-bridge.cmd
```

Console attendue :

```
============================================
  IP PC pour la 3DS : 192.168.x.x
  Port UDP : 5000
============================================
Listening UDP :5000
Spout sender: 3DS2SPOUT
```

---

## 3. Source Spout

1. Panneau **Sources** → **Spout** → **3DS2SPOUT**
2. Glisser sur **Layer 1, Column 1**

> La source n’apparaît qu’après au moins une frame (test pattern au démarrage du bridge).

---

## 4. Chaîne d’effets recommandée (Layer 1)

| Ordre | Effet | Param OSC |
|-------|-------|-----------|
| 1 | **Blur** | Opacité effet 1 ← slider (0x10) |
| 2 | **Kaleidoscope** | Preset 2 (0x21) |
| 3 | **Colorize** | Preset 3 (0x22) |
| 4 | **Transform** | Blend mode (0x30) |

---

## 5. Démarrer le stream 3DS

1. 3DS : **IP PC** configurée (voir ordre exact ci-dessus)
2. Lance **3DS2SPOUT**
3. Appuie sur **REC**
4. Vérifie **PC OK** + FPS sur l’écran du bas
5. Vérifie le compteur FPS dans la console du bridge

---

## 6. Mapping tactile → OSC

| Contrôle 3DS | param_id | Cible Resolume |
|--------------|----------|----------------|
| Slider intensité | `0x10` | Layer 1 / Effect 1 opacity |
| Presets 1–6 | `0x20`–`0x25` | Effects 1–6 enabled |
| Blend cycle | `0x30` | Layer 1 blend mode |

Mappings : [`osc-map.toml`](osc-map.toml) — passer `--config` au bridge Rust.

---

## Dépannage

| Problème | Solution |
|----------|----------|
| Pas de source Spout | Bridge lancé ? DLL Spout OK ? Attendre une frame |
| Pas d’image | REC actif ? IP PC correcte (bouton **IP PC**) ? |
| Pas d’OSC | Port 7000 ouvert ? `python-osc` installé ? |
