# 3DS2SPOUT (homebrew 3DS)

Capture caméra + interface tactile + stream QOI/UDP vers le bridge PC.

---

## Prérequis (compilation)

| Élément | Installation |
|---------|----------------|
| [devkitPro](https://devkitpro.org/wiki/Getting_Started) | Installateur Windows |
| **MSYS2 devkitPro** | Shell pour compiler |
| Groupe **3ds-dev** | `pacman -S --needed --noconfirm 3ds-dev` |

> À l’invite pacman `Enter a selection`, appuie **Entrée** (default=all).  
> Ne tape pas d’autres commandes dans le prompt pacman.

---

## Compiler

```bash
cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream
make clean && make
```

**Sortie :** `3ds-cam-stream.3dsx` (titre **3DS2SPOUT**, icône dans le Homebrew Launcher)

Outils devkitPro utilisés :

| Outil | Rôle |
|-------|------|
| `smdhtool --create` | Fichier `.smdh` (icône + titre) |
| `3dsxtool --smdh=` | Package `.3dsx` |

Icône : [`gfx/icon.png`](gfx/icon.png) — regénérer : `python gfx/generate-icon.py`

---

## Installer sur la 3DS

### Utilisateur (release)

1. Copie `3ds-cam-stream.3dsx` → `SD:/3ds/`
2. Configure l’IP via le bouton **IP PC** sur la console (voir ci-dessous)

### Développeur (WiFi)

Depuis la **racine** `apps/` :

```bash
cd /d/CREA/TOOLS/3DS/apps
./scripts/install-to-3ds.sh IP_3DS
```

Windows : `install\install-3ds.cmd`

---

## Config IP PC (écran tactile)

**Méthode recommandée** — sans éditer la SD sur PC :

1. PC : `install\run-bridge.cmd` → note l’**IP PC** dans la console
2. 3DS : bouton **IP PC** (bas gauche) → clavier `0-9` `.` **DEL** → **OK**
3. Sauvegarde auto : `sdmc:/3ds-cam-stream.cfg`
4. **B** ou **ANNUL** pour annuler

**Langue / Language** : sur ce même écran, le bouton **LANGUE FR / LANG EN** (haut droite) bascule toute l'interface entre français et anglais. Persisté dans le `.cfg` (`lang=fr` ou `lang=en`).

---

## Interface tactile (écran du bas)

| Élément | Action |
|---------|--------|
| **REC** | Start/stop stream — affiche `PC OK` + FPS |
| **EXTERNE / INTERNE** | Caméra arrière / avant |
| **FLUIDE 200P / HD 400P** | Qualité du flux |
| **IP PC** | Saisie IP du PC + bouton langue FR/EN |
| Ligne info | IP 3DS + IP:port PC |

Raccourcis : **A** = REC, **Y/X** = caméra, **START** = quitter.

---

## Release GitHub

```bash
make release VERSION=1.0.0 RELEASE_PC_IP=192.168.0.1
```

Windows : `scripts\make-release.cmd 1.0.0`

---

## Optionnel : menu HOME (CIA)

Forwarder CIA recommandé — [`docs/FORWARDER-FBI.md`](../docs/FORWARDER-FBI.md)

---

## Voir aussi

- [`../README.md`](../README.md) — quick start complet
- [`../install/README.md`](../install/README.md) — scripts Windows
