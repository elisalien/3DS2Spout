# 3DS2SPOUT — Camera Stream → Resolume

Stream the Nintendo 3DS camera over WiFi to a Windows PC bridge, output via **Spout** into **Resolume Arena**, with touch-controlled effects on the bottom screen and OSC relay for heavy Resolume effects.

---

## Quick start — ordre exact

> **Une seule IP à configurer : celle du PC.**  
> L’IP de la 3DS est affichée à l’écran (info). Tu la configures **sur la console** (bouton **IP PC**), sans éditer de fichier sur PC.

### Prérequis

#### Matériel & logiciels (obligatoire)

| Élément | Détail |
|---------|--------|
| **PC Windows 10/11** | Même réseau **WiFi** que la 3DS |
| **Python 3.10+** | [python.org](https://www.python.org/downloads/) — cocher **Add to PATH** |
| **Resolume Arena** | Entrée source **Spout** |
| **Nintendo 3DS** | **Luma3DS** + **Homebrew Launcher** |

> **SpoutLibrary.dll** est téléchargée et installée **automatiquement** par `install\INSTALL.cmd` (Spout 2.007.017, empreinte SHA256 vérifiée). En cas d'erreur Spout au démarrage : installer le [Visual C++ Redistributable x64](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).

#### Optionnel

| Élément | Quand |
|---------|--------|
| **Rust / cargo** | Bridge plus rapide ; **Python suffit** (`INSTALL.cmd -WithRust`) |
| **devkitPro + MSYS2** | Uniquement si tu **compiles** le `.3dsx` toi-même |
| **FTPD** (3DS) | Déploiement WiFi sans retirer la SD (dev) |
| **Forwarder CIA + FBI** | Icône sur le **menu HOME** Nintendo — voir [`docs/FORWARDER-FBI.md`](docs/FORWARDER-FBI.md) |

Vérification automatique : `install\check-prerequisites.cmd`

---

### Installation — utilisateur (release, sans compiler)

| # | Où | Action |
|---|-----|--------|
| **1** | PC | Télécharge le zip **3DS2SPOUT-x.x.x** (GitHub Releases) **ou** clone ce dépôt |
| **2** | PC | Double-clic `install\INSTALL.cmd` — packages Python **et SpoutLibrary.dll automatiques** |
| **3** | 3DS | Copie **une seule fois** `3ds-cam-stream.3dsx` → `SD:/3ds/` |
| **4** | PC | Lance `install\run-bridge.cmd` — **note l’IP affichée** dans la console |
| **5** | 3DS | Homebrew Launcher → **3DS2SPOUT** → bouton **IP PC** → saisir l’IP → **OK** |
| **6** | PC | Ouvre **Resolume Arena** → source Spout **3DS2SPOUT** sur Layer 1 ([`resolume/SETUP.md`](resolume/SETUP.md)) |
| **7** | 3DS | Appuie sur **REC** — vérifie **PC OK** + FPS en bas d’écran |

---

### Installation — développeur (compiler + déployer)

| # | Où | Action |
|---|-----|--------|
| **1–2** | PC | Comme ci-dessus (`INSTALL.cmd`) |
| **4** | PC | Installe [devkitPro](https://devkitpro.org/wiki/Getting_Started) → shell **MSYS2 devkitPro** |
| **5** | PC | `pacman -S --needed --noconfirm 3ds-dev` (puis **Entrée** si demandé) |
| **6** | PC | Compile : |
| | | `cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream` |
| | | `make clean && make` |
| **7** | 3DS | Lance **FTPD**, note l’IP de la console |
| **8** | PC | Déploie (depuis la **racine** `apps/`, pas `3ds-cam-stream/`) : |
| | | `cd /d/CREA/TOOLS/3DS/apps` |
| | | `./scripts/install-to-3ds.sh 192.168.1.45` |
| | | ou `.\scripts\install-to-3ds.ps1 -ThreeDSIp 192.168.1.45` |
| **9–10** | | Puis étapes **4–7** du parcours utilisateur (bridge → IP PC → Resolume → REC) |

**Build release GitHub :**

```cmd
scripts\make-release.cmd 1.0.0
```

---

## Config IP PC (écran tactile 3DS)

1. PC : `install\run-bridge.cmd` affiche :

   ```
   ============================================
     IP PC pour la 3DS : 192.168.x.x
     Sur la 3DS : bouton IP PC -> saisir ...
     Port UDP : 5000
   ============================================
   ```

2. 3DS : bouton **IP PC** (bas gauche) → clavier `0-9` / `.` / **DEL** → **OK**
3. L’IP est enregistrée dans `sdmc:/3ds-cam-stream.cfg` — **pas besoin de retirer la SD**
4. **B** ou **ANNUL** pour annuler

---

## Point d’entrée installation

| Fichier | Rôle |
|---------|------|
| [`install/INSTALL.cmd`](install/INSTALL.cmd) | Installation guidée PC |
| [`install/install-dependencies.cmd`](install/install-dependencies.cmd) | Dépendances Python |
| [`install/install-3ds.cmd`](install/install-3ds.cmd) | Assistant déploiement 3DS (WiFi) |
| [`install/run-bridge.cmd`](install/run-bridge.cmd) | Lance le bridge PC |
| [`install/check-prerequisites.cmd`](install/check-prerequisites.cmd) | Diagnostic prérequis |
| [`install/guide/GUIDE-DEBUTANT.pdf`](install/guide/GUIDE-DEBUTANT.pdf) | Guide PDF pas à pas |

Voir [`install/README.md`](install/README.md).

---

## Site web (GitHub Pages)

Page bilingue FR/EN : [`docs/index.html`](docs/index.html)  
Activer : GitHub **Settings → Pages →** branche `main`, dossier **`/docs`**.

---

## Composants

| Dossier | Rôle |
|---------|------|
| [`3ds-cam-stream/`](3ds-cam-stream/) | Homebrew 3DSX — capture, UI tactile, stream QOI/UDP |
| [`pc-bridge/`](pc-bridge/) | Bridge UDP → QOI → Spout + relais OSC (Rust ou Python) |
| [`resolume/`](resolume/) | Config Resolume Arena + mapping OSC |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Protocole et flux de données |
| [`docs/FORWARDER-FBI.md`](docs/FORWARDER-FBI.md) | Menu HOME + distribution `.3dsx` |

---

## Contrôles tactiles (écran du bas)

| Contrôle | Action |
|----------|--------|
| **REC** | Démarre / arrête le stream (`PC OK` + FPS si connecté) |
| **EXTERNE / INTERNE** | Caméra arrière / avant |
| **FLUIDE 200P / HD 400P** | Qualité du flux |
| **IP PC** | Saisie tactile de l’IP du PC |
| **A** | REC (raccourci) |
| **Y / X** | Caméra externe / interne |
| **START** | Quitter |

---

## Bridge PC

```cmd
cd /d D:\CREA\TOOLS\3DS\apps
install\run-bridge.cmd
```

Rust si disponible, sinon Python automatiquement. Autorise **UDP entrant port 5000** dans le pare-feu Windows.

---

## Protocole UDP (port 5000)

| Octets | Type |
|--------|------|
| `0xFC 0x00` | Handshake |
| `0xFC 0x01` | Frame QOI |
| `0xFC 0x02` | Contrôle OSC (param_id + float) |

Détails : [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

---

## Licence

MIT — encodeur QOI basé sur [phoboslab/qoi](https://github.com/phoboslab/qoi) (MIT).
