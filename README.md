# 3DS2SPOUT

> La caméra de la Nintendo 3DS en source Spout — sans fil, pour le VJing et les visuels live.

**Français** · [English](README.en.md)

![Platform](https://img.shields.io/badge/platform-Nintendo%203DS%20%7C%20Windows-informational) ![Stack](https://img.shields.io/badge/stack-C%20%C2%B7%20Python%20%C2%B7%20Rust-blue) ![Output](https://img.shields.io/badge/output-Spout%20%2B%20OSC-orange) ![License](https://img.shields.io/badge/license-MIT-green)

**[Télécharger la dernière release](https://github.com/elisalien/3ds2spout/releases/latest)** · **[Guide débutant (PDF)](install/guide/GUIDE-DEBUTANT.pdf)**

<p align="center"><img src="3ds-cam-stream/gfx/icon.png" width="96" alt="Icône 3DS2SPOUT"></p>

## À propos

3DS2SPOUT transforme une 3DS en caméra WiFi pour le VJing et les visuels live : l'image est envoyée vers un PC Windows, qui la publie comme source **Spout** utilisable dans n'importe quel logiciel compatible (Resolume, TouchDesigner, OBS, MadMapper…). L'écran tactile de la console sert de télécommande : changement de caméra, qualité, effets relayés en **OSC**. Interface FR/EN.

```
3DS (caméra + écran tactile)  ──WiFi/UDP──▶  PC (bridge)  ──Spout──▶  ton logiciel visuel
                                                          ──OSC───▶  contrôle d'effets
```

Aucun câble, aucun fichier à éditer : la seule configuration est l'IP du PC, saisie une fois sur l'écran tactile de la 3DS.

## Fonctionnalités

- Stream de la caméra 3DS (arrière ou avant) en WiFi/UDP, images encodées en QOI
- Publication côté PC comme source Spout **3DS2SPOUT**
- Écran tactile comme télécommande : REC, choix de caméra, qualité (FLUIDE 200P / HD 400P), effets relayés en OSC
- Bridge PC en Python, ou en Rust (plus rapide) s'il est disponible — choix automatique
- Installation automatisée sous Windows (packages, Spout) et guide PDF illustré
- Interface FR/EN

## Installation

Guide illustré pas à pas : [`install/guide/GUIDE-DEBUTANT.pdf`](install/guide/GUIDE-DEBUTANT.pdf)

Il faut : un PC Windows 10/11 avec [Python 3.10+](https://www.python.org/downloads/) et un logiciel compatible Spout, une 3DS avec [Luma3DS + Homebrew Launcher](https://3ds.hacks.guide), les deux sur le même WiFi. Le reste (packages, Spout) s'installe automatiquement. Le pare-feu doit autoriser l'UDP entrant sur le port **5000**.

| # | Où | Étape |
|---|-----|-------|
| 1 | PC | Télécharger la [release](https://github.com/elisalien/3ds2spout/releases) et lancer `install\INSTALL.cmd` |
| 2 | 3DS | Copier `3ds-cam-stream.3dsx` dans le dossier `3ds` de la carte SD (une fois) |
| 3 | PC | Lancer `install\run-bridge.cmd` — l'IP à utiliser s'affiche |
| 4 | 3DS | Ouvrir **3DS2SPOUT** → bouton **IP PC** → saisir l'IP → **OK** |
| 5 | | Dans le logiciel visuel : source Spout **3DS2SPOUT**. Sur la 3DS : **REC** |

Détails, options et dépannage : [`install/README.md`](install/README.md)

## Utilisation

### Contrôles (écran tactile)

| Contrôle | Action |
|----------|--------|
| **REC** | Démarre / arrête le stream (`PC OK` + FPS si connecté) |
| **EXTERNE / INTERNE** | Caméra arrière / avant (raccourcis **Y** / **X**) |
| **FLUIDE 200P / HD 400P** | Qualité du flux |
| **IP PC** | IP du PC + langue FR/EN |
| **START** | Quitter |

L'IP peut aussi être fournie via un fichier `SD:/3ds-cam-stream.cfg` (voir [`3ds-cam-stream/example.cfg`](3ds-cam-stream/example.cfg) : `pc_ip`, `pc_port`, `lang`).

### Resolume

Mise en place dans Resolume Arena (Spout + OSC) : [`resolume/SETUP.md`](resolume/SETUP.md), mapping OSC dans [`resolume/osc-map.toml`](resolume/osc-map.toml). La cible OSC du bridge se règle dans [`pc-bridge/config.toml`](pc-bridge/config.toml).

### Pour les développeurs

Compilation du `.3dsx` avec devkitPro (`pacman -S 3ds-dev`, puis `make` dans `3ds-cam-stream/`), déploiement WiFi via FTPD (`scripts/install-to-3ds.ps1` ou `.sh`), build de release avec `scripts/make-release.cmd`. Le protocole UDP (handshake, frames QOI, contrôle OSC — port 5000) est décrit dans [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md), le déploiement dans [`scripts/README.md`](scripts/README.md), le bridge dans [`pc-bridge/README.md`](pc-bridge/README.md).

## Structure du projet

| Dossier | Rôle |
|---------|------|
| [`3ds-cam-stream/`](3ds-cam-stream/) | Application 3DS — capture, UI tactile, stream QOI/UDP |
| [`pc-bridge/`](pc-bridge/) | Bridge PC : UDP → Spout + relais OSC (Python, ou Rust) |
| [`resolume/`](resolume/) | Exemple d'intégration Resolume + mapping OSC |
| [`install/`](install/) | Scripts d'installation et guide PDF |
| [`scripts/`](scripts/) | Déploiement 3DS, installation Rust/bridge, build de release |
| [`docs/`](docs/) | [Architecture & protocole](docs/ARCHITECTURE.md), [icône menu HOME](docs/FORWARDER-FBI.md) |

## Stack technique

C (devkitPro / libctru) · QOI · UDP · Python (`qoi`, `python-osc`) · Rust · Spout2 (`SpoutLibrary.dll`) · OSC · PowerShell / batch

## Crédits & licence

Encodeur QOI basé sur [phoboslab/qoi](https://github.com/phoboslab/qoi) (MIT). Sortie Spout via [Spout2](https://github.com/leadedge/Spout2).

Licence [MIT](LICENSE).

---

<p align="center">
  Fait par <a href="https://github.com/elisalien">Élisalien</a> ·
  <a href="https://gitlab.com/elisalien/catalog">Catalogue des shaders, sources & logiciels</a>
</p>
