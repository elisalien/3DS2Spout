# 3DS2SPOUT

**Français** · [English](README.en.md)

**La caméra de la Nintendo 3DS en source Spout — sans fil.**

3DS2SPOUT transforme une 3DS en caméra WiFi pour le VJing et les visuels live :
l'image est envoyée vers un PC Windows, qui la publie comme source **Spout**
utilisable dans n'importe quel logiciel compatible (Resolume, TouchDesigner,
OBS, MadMapper…). L'écran tactile de la console sert de télécommande :
changement de caméra, qualité, effets relayés en **OSC**. Interface FR/EN.

---

## Fonctionnement

```
3DS (caméra + écran tactile)  ──WiFi/UDP──▶  PC (bridge)  ──Spout──▶  ton logiciel visuel
                                                          ──OSC───▶  contrôle d'effets
```

Aucun câble, aucun fichier à éditer : la seule configuration est l'IP du PC,
saisie une fois sur l'écran tactile de la 3DS.

---

## Installation

Guide illustré pas à pas : [`install/guide/GUIDE-DEBUTANT.pdf`](install/guide/GUIDE-DEBUTANT.pdf)

Il faut : un PC Windows 10/11 avec [Python 3.10+](https://www.python.org/downloads/) et
un logiciel compatible Spout, une 3DS avec
[Luma3DS + Homebrew Launcher](https://3ds.hacks.guide), les deux sur le même WiFi.
Le reste (packages, Spout) s'installe automatiquement.

| # | Où | Étape |
|---|-----|-------|
| 1 | PC | Télécharger la [release](../../releases) et lancer `install\INSTALL.cmd` |
| 2 | 3DS | Copier `3ds-cam-stream.3dsx` dans le dossier `3ds` de la carte SD (une fois) |
| 3 | PC | Lancer `install\run-bridge.cmd` — l'IP à utiliser s'affiche |
| 4 | 3DS | Ouvrir **3DS2SPOUT** → bouton **IP PC** → saisir l'IP → **OK** |
| 5 | | Dans le logiciel visuel : source Spout **3DS2SPOUT**. Sur la 3DS : **REC** |

Détails, options et dépannage : [`install/README.md`](install/README.md)

---

## Contrôles (écran tactile)

| Contrôle | Action |
|----------|--------|
| **REC** | Démarre / arrête le stream (`PC OK` + FPS si connecté) |
| **EXTERNE / INTERNE** | Caméra arrière / avant (raccourcis **Y** / **X**) |
| **FLUIDE 200P / HD 400P** | Qualité du flux |
| **IP PC** | IP du PC + langue FR/EN |
| **START** | Quitter |

---

## Contenu du dépôt

| Dossier | Rôle |
|---------|------|
| [`3ds-cam-stream/`](3ds-cam-stream/) | Application 3DS — capture, UI tactile, stream QOI/UDP |
| [`pc-bridge/`](pc-bridge/) | Bridge PC : UDP → Spout + relais OSC (Python, ou Rust) |
| [`resolume/`](resolume/) | Exemple d'intégration Resolume + mapping OSC |
| [`install/`](install/) | Scripts d'installation et guide PDF |
| [`docs/`](docs/) | [Architecture & protocole](docs/ARCHITECTURE.md), [icône menu HOME](docs/FORWARDER-FBI.md) |

---

## Pour les développeurs

Compilation du `.3dsx` avec devkitPro (`pacman -S 3ds-dev`, puis `make` dans
`3ds-cam-stream/`), déploiement WiFi via FTPD (`scripts/install-to-3ds.ps1` ou `.sh`),
build de release avec `scripts/make-release.cmd`. Le protocole UDP (handshake, frames
QOI, contrôle OSC — port 5000) est décrit dans
[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md), le déploiement dans
[`scripts/README.md`](scripts/README.md).

---

## Licence

MIT — encodeur QOI basé sur [phoboslab/qoi](https://github.com/phoboslab/qoi) (MIT).
