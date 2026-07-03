# Scripts d’installation 3DS2SPOUT

Scripts pour déployer le homebrew sur la 3DS **via WiFi** (sans retirer la SD du PC).

---

## Prérequis

### Sur la 3DS

| Élément | Détail |
|---------|--------|
| **Luma3DS** + Homebrew Launcher | CFW fonctionnel |
| **FTPD** (ou ftpserv) | **Lancé** pendant le déploiement |
| IP affichée | Ex. `192.168.1.45` |
| Port FTP | **5000** (défaut) |
| Utilisateur | `anonymous` (mot de passe souvent vide ou `1234`) |

### Sur le PC

| Élément | Détail |
|---------|--------|
| **devkitPro** + MSYS2 | `make` et `3dslink` disponibles |
| **3ds-dev** installé | `pacman -S --needed --noconfirm 3ds-dev` |
| **Homebrew compilé** | `make` dans `3ds-cam-stream/` |
| Même WiFi | Que la 3DS |

> **Chemin important :** les scripts sont dans `apps/scripts/`.  
> Exécute-les depuis **`apps/`** (racine), **pas** depuis `3ds-cam-stream/`.

---

## Ordre exact — déploiement WiFi

```
1. MSYS2 devkitPro :
   cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream
   make clean && make

2. Sur la 3DS : lance FTPD, note l’IP (ex. 192.168.1.45)

3. MSYS2 devkitPro :
   cd /d/CREA/TOOLS/3DS/apps
   ./scripts/install-to-3ds.sh 192.168.1.45

4. PC : install\run-bridge.cmd (noter l’IP PC affichée)

5. 3DS : bouton IP PC → saisir l’IP → OK → REC
```

---

## Windows (PowerShell)

Depuis la **racine** `apps/` :

```powershell
cd D:\CREA\TOOLS\3DS\apps

# Compile + copie .3dsx sur SD + lance l’app
.\scripts\install-to-3ds.ps1 -ThreeDSIp 192.168.1.45

# Mot de passe FTP si demandé
.\scripts\install-to-3ds.ps1 -ThreeDSIp 192.168.1.45 -FtpPassword 1234

# Build + 3dslink seulement (sans écrire sur SD)
.\scripts\install-to-3ds.ps1 -ThreeDSIp 192.168.1.45 -SkipFtp
```

Équivalent double-clic : `install\install-3ds.cmd`

---

## MSYS2 / devkitPro (bash)

**Syntaxe des chemins** — en bash MSYS, pas de `cd /d D:\...` :

```bash
cd /d/CREA/TOOLS/3DS/apps
./scripts/install-to-3ds.sh 192.168.1.45
# ou avec IP PC explicite :
./scripts/install-to-3ds.sh 192.168.1.45 192.168.1.42
```

Raccourci préconfiguré : `./scripts/install-my-3ds.sh`

---

## Ce que fait le script

1. Détecte l’**IP LAN du PC** (ou utilise celle passée en argument)
2. Compile le homebrew si nécessaire (`make PC_IP=...`)
3. **FTP** → `sdmc:/3ds/3ds-cam-stream.3dsx` (+ `.cfg` optionnel)
4. **3dslink** → lance l’app (Homebrew Launcher ouvert, **Y** = NetLoader)

---

## Config IP après déploiement

Le script peut écrire un `.cfg` initial, mais la méthode recommandée est :

1. `install\run-bridge.cmd` → lire l’IP PC dans la console
2. Sur la 3DS : **IP PC** → saisie tactile → **OK**

Pas besoin de rééditer le `.cfg` sur PC.

---

## 3dslink manuel

1. 3DS : Homebrew Launcher → **Y** (NetLoader)
2. PC :

```bash
cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream
3dslink -a IP_3DS 3ds-cam-stream.3dsx
```

Windows cmd : `launch-3ds.cmd`

---

## Bridge PC

**Utilise cmd/PowerShell**, pas MSYS (cargo souvent absent) :

```cmd
cd /d D:\CREA\TOOLS\3DS\apps
install\run-bridge.cmd
```

---

## Compilation (devkitPro actuel)

Outils utilisés par le Makefile :

| Outil | Rôle |
|-------|------|
| `smdhtool --create` | Icône + métadonnées `.smdh` |
| `3dsxtool ... --smdh=` | Génère le `.3dsx` |

```bash
cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream
make clean && make
```

---

## Dépannage

| Problème | Solution |
|----------|----------|
| `./scripts/...` introuvable | Tu es dans `3ds-cam-stream/` → remonte : `cd ..` |
| `make` introuvable | Ouvre **MSYS2 devkitPro**, pas PowerShell |
| `bannertool` introuvable | Normal — le Makefile utilise **smdhtool** |
| `pacman` demande une sélection | Appuie **Entrée** seulement (default=all) |
| FTP échoue | FTPD actif, même WiFi, `-FtpPassword 1234` |
| `3dslink` échoue | HBL ouvert → **Y** (NetLoader) |
| Pas de connexion PC | Bouton **IP PC** sur 3DS, vérifier IP bridge |
| `cargo` introuvable (MSYS) | Bridge depuis **cmd** : `run-bridge.cmd` |

---

## Fichiers sur la SD

| Chemin SD | Contenu |
|-----------|---------|
| `3ds/3ds-cam-stream.3dsx` | Homebrew **3DS2SPOUT** |
| `3ds-cam-stream.cfg` | Créé auto par saisie tactile **IP PC** (optionnel) |
