# Installation 3DS2SPOUT

Dossier d'entrée pour installer et lancer le projet.

> **Débutant ?** Suis simplement le [guide PDF pas à pas](guide/GUIDE-DEBUTANT.pdf) — 4 étapes.

---

## Ce qu'il te faut

| Élément | Détail |
|---------|--------|
| PC Windows 10/11 | Même WiFi que la 3DS |
| Resolume Arena | Source Spout |
| Python 3.10+ | [python.org](https://www.python.org/downloads/) — cocher **Add to PATH** |
| 3DS + Luma3DS + Homebrew Launcher | 3DS modée ([3ds.hacks.guide](https://3ds.hacks.guide)) |

Tout le reste (packages Python, **SpoutLibrary.dll**) est installé automatiquement par `INSTALL.cmd`.

**Diagnostic :** `check-prerequisites.cmd`

---

## Installation en 4 étapes

```
1. PC   → install\INSTALL.cmd                    (tout est automatique)
2. 3DS  → Copier 3ds-cam-stream.3dsx → SD:/3ds/  (une seule fois)
3. PC   → install\run-bridge.cmd                 (noter l'IP affichée)
        → 3DS : 3DS2SPOUT → bouton IP PC → saisir IP → OK
4. PC   → Resolume → source Spout 3DS2SPOUT — 3DS : REC
```

> Pas de fichier `.cfg` à éditer. L'IP se configure **sur la 3DS** (sauvegarde auto).
> Le bouton **LANGUE FR / LANG EN** (écran IP PC) bascule l'interface en anglais.

---

## Scripts (double-clic Windows)

| Action | Fichier |
|--------|---------|
| Installation PC complète | [`INSTALL.cmd`](INSTALL.cmd) |
| Dépendances seulement | [`install-dependencies.cmd`](install-dependencies.cmd) |
| Vérifier prérequis | [`check-prerequisites.cmd`](check-prerequisites.cmd) |
| Lancer le bridge | [`run-bridge.cmd`](run-bridge.cmd) |
| Guide PDF | [`guide/GUIDE-DEBUTANT.pdf`](guide/GUIDE-DEBUTANT.pdf) |

Options : `INSTALL.cmd -WithRust` installe aussi le bridge Rust (facultatif, Python suffit).

---

## Développeurs (compiler + déployer en WiFi)

```
1. devkitPro MSYS2 : pacman -S --needed --noconfirm 3ds-dev
2. cd 3ds-cam-stream && make clean && make
3. 3DS : lancer FTPD
4. Depuis la racine apps/ :
   ./scripts/install-to-3ds.sh IP_3DS        (ou install\install-3ds.cmd)
```

Détails : [`../scripts/README.md`](../scripts/README.md)

**Release GitHub :** `scripts\make-release.cmd 1.0.0`

**Régénérer le guide PDF :** `python install\generate-guide-pdf.py`

---

## Voir aussi

- [`../README.md`](../README.md) — vue d'ensemble
- [`../docs/FORWARDER-FBI.md`](../docs/FORWARDER-FBI.md) — icône menu HOME + FBI
- [`../resolume/SETUP.md`](../resolume/SETUP.md) — config Resolume détaillée
