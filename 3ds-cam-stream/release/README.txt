3DS2SPOUT - Guide rapide (release .3dsx)
=========================================

PREREQUIS
---------
- PC Windows 10/11 + Python 3.10+
- SpoutLibrary.dll x64 dans pc-bridge/vendor/ (sur le PC)
- Resolume Arena
- 3DS avec Luma3DS + Homebrew Launcher
- PC et 3DS sur le MEME WiFi

INSTALLATION (ordre exact)
--------------------------

1. PC : install\INSTALL.cmd
2. PC : copier SpoutLibrary.dll -> pc-bridge\vendor\
3. 3DS : copier 3ds-cam-stream.3dsx -> SD:/3ds/     (une seule fois)
4. PC : install\run-bridge.cmd
        -> noter l'IP affichee :
           "IP PC pour la 3DS : 192.168.x.x"
5. 3DS : Homebrew Launcher -> 3DS2SPOUT
        -> bouton IP PC -> saisir l'IP -> OK
6. PC : Resolume -> source Spout 3DS2SPOUT
7. 3DS : REC (verifier PC OK + FPS)

CONFIG IP
---------
- Pas besoin d'editer un fichier .cfg sur PC
- L'IP se configure sur la 3DS (bouton IP PC)
- Sauvegarde auto : SD:/3ds-cam-stream.cfg

MENU HOME (optionnel)
---------------------
Voir docs/FORWARDER-FBI.md (forwarder CIA + FBI)

DOCUMENTATION
-------------
install/guide/GUIDE-DEBUTANT.pdf
README.md
