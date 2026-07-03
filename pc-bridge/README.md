# PC Bridge

Récepteur UDP : décode QOI → sortie **Spout** (Resolume) + relais **OSC**.

---

## Prérequis

| Élément | Détail |
|---------|--------|
| **Windows 10/11** | Même WiFi que la 3DS |
| **Python 3.10+** | Obligatoire (fallback si Rust indisponible) |
| **SpoutLibrary.dll** (x64) | `vendor/SpoutLibrary.dll` — [Spout2](https://github.com/leadedge/Spout2/releases) |
| **Pare-feu** | UDP entrant **port 5000** |
| **Rust** (optionnel) | Bridge plus rapide — `cargo build --release` |

Packages Python : `pip install -r requirements.txt` (ou `install\install-dependencies.cmd`)

---

## Lancer (recommandé)

```cmd
cd /d D:\CREA\TOOLS\3DS\apps
install\run-bridge.cmd
```

Rust si disponible, sinon Python automatiquement.

---

## IP affichée pour la 3DS

Au démarrage, la console affiche :

```
============================================
  IP PC pour la 3DS : 192.168.x.x
  Sur la 3DS : bouton IP PC -> saisir 192.168.x.x
  Port UDP : 5000
============================================
```

Saisis cette IP sur la 3DS (bouton **IP PC**). **Aucune config PC supplémentaire** pour l’adresse 3DS — le bridge écoute sur toutes les interfaces.

---

## Spout

Copie `SpoutLibrary.dll` (x64 **MD**) dans `vendor/` :

```
Spout-SDK-binaries → SpoutLibrary/Binaries/SpoutLibrary_x64/SpoutLibrary.dll
```

Sans DLL : décodage + log FPS, mais **pas de Spout** dans Resolume.

Voir [`vendor/README.txt`](vendor/README.txt).

---

## Run manuel

**Python :**

```cmd
cd pc-bridge
python receiver.py
```

**Rust :**

```powershell
cargo run --release
cargo run --release -- --config ../resolume/osc-map.toml
```

---

## Ordre dans le workflow global

```
1. Lancer le bridge (ce module)
2. Noter l'IP PC affichée
3. 3DS : IP PC → saisir l'IP → OK
4. 3DS : REC
5. Resolume : source Spout 3DS2SPOUT
```

Voir [`../README.md`](../README.md).
