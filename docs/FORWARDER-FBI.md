# Distribution .3dsx + menu HOME (FBI)

Guide **3DS2SPOUT** — installation homebrew et icône optionnelle sur le menu HOME Nintendo.

---

## Prérequis

| Élément | Obligatoire | Détail |
|---------|-------------|--------|
| Luma3DS + Homebrew Launcher | Oui | CFW sur la 3DS |
| PC Windows + Python 3.10+ | Oui | Bridge Spout |
| SpoutLibrary.dll (x64) | Oui | `pc-bridge/vendor/` |
| Resolume Arena | Oui | Source Spout |
| Même WiFi | Oui | PC ↔ 3DS |
| FBI | Optionnel | Menu HOME (forwarder CIA) |
| devkitPro | Optionnel | Compiler le `.3dsx` soi-même |

---

## Ordre d’installation exact (utilisateur release)

```
1. PC  → install\INSTALL.cmd
2. PC  → SpoutLibrary.dll → pc-bridge\vendor\
3. 3DS → 3ds-cam-stream.3dsx → SD:/3ds/          (une fois)
4. PC  → install\run-bridge.cmd                   (noter IP PC)
5. 3DS → 3DS2SPOUT → IP PC → saisir IP → OK
6. PC  → Resolume → Spout 3DS2SPOUT
7. 3DS → REC
```

> **Pas de `.cfg` à éditer sur PC.** L’IP se configure sur la 3DS (bouton **IP PC**).

---

## Méthode officielle : `.3dsx`

### Fichier à installer

| Fichier | Destination |
|---------|-------------|
| `3ds-cam-stream.3dsx` | `SD:/3ds/3ds-cam-stream.3dsx` |

Depuis une release GitHub (`3DS2SPOUT-x.x.x.zip`) ou `make release`.

Le `.cfg.example` du zip est **optionnel** — la saisie tactile **IP PC** le remplace.

### Configurer l’IP du PC

**Recommandé :**

1. PC : `install\run-bridge.cmd` → lire `IP PC pour la 3DS : …`
2. 3DS : **IP PC** → clavier tactile → **OK**

**Alternative manuelle** — `SD:/3ds-cam-stream.cfg` :

```ini
pc_ip=192.168.1.42
pc_port=5000
```

---

## Menu HOME Nintendo (optionnel)

Le `.3dsx` se lance depuis le **Homebrew Launcher**, pas la grille HOME.

| Option | Recommandation |
|--------|----------------|
| **Forwarder CIA** | ✅ Facile à mettre à jour |
| **CIA complète** (`make cia`) | Avancé — makerom souvent absent |

### Forwarder CIA + FBI

1. `.3dsx` déjà dans `SD:/3ds/`
2. Crée un forwarder → `sdmc:/3ds/3ds-cam-stream.3dsx`
3. Installe la `.cia` avec **FBI**
4. Mises à jour : remplace seulement le `.3dsx`

Paramètres forwarder :

| Champ | Valeur |
|-------|--------|
| Titre | `3DS2SPOUT` |
| Chemin | `sdmc:/3ds/3ds-cam-stream.3dsx` |

---

## Compiler (développeur)

```bash
# MSYS2 devkitPro
pacman -S --needed --noconfirm 3ds-dev

cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream
make clean && make

# Déployer (depuis apps/, pas 3ds-cam-stream/)
cd /d/CREA/TOOLS/3DS/apps
./scripts/install-to-3ds.sh IP_3DS
```

Outils : **smdhtool** + **3dsxtool --smdh=** (plus `bannertool`).

### Release GitHub

```cmd
scripts\make-release.cmd 1.0.0
```

---

## Dépannage

| Problème | Solution |
|----------|----------|
| Pas de connexion PC | **IP PC** sur 3DS ; bridge lancé ; même WiFi |
| Forwarder → écran noir | Vérifier `SD:/3ds/3ds-cam-stream.3dsx` |
| `./scripts/...` introuvable | Exécuter depuis `apps/`, pas `3ds-cam-stream/` |
| `make` / smdhtool | Shell **MSYS2 devkitPro** |

---

## Voir aussi

- [`../README.md`](../README.md)
- [`../install/README.md`](../install/README.md)
- [`../install/guide/GUIDE-DEBUTANT.pdf`](../install/guide/GUIDE-DEBUTANT.pdf)
