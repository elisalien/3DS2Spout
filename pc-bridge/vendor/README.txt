SpoutLibrary.dll pour le PC bridge uniquement (Resolume n'en a pas besoin).

1. Telecharge : https://github.com/leadedge/Spout2/releases
   Fichier : Spout-SDK-binaries_XXX.zip

2. Copie la DLL **x64** (pas Win32) :
   SpoutLibrary/Binaries/SpoutLibrary_x64/SpoutLibrary.dll
   -> pc-bridge/vendor/SpoutLibrary.dll

3. Version runtime : **MD** (Multi-threaded DLL) — c'est la version officielle distribuee.
   Ne mets PAS une DLL MT compilee maison sauf si tu rebuilds tout en MT.
   Python 3.10 64-bit = x64 MD.

4. Installe aussi le Visual C++ Redistributable x64 si CreateOpenGL echoue :
   https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

5. Resolume : onglet **Sources** (pas Effects) -> **Spout** -> **3DS2SPOUT**
   La source n'apparait qu'apres au moins une frame envoyee (le bridge envoie un test au demarrage).

Ne copie pas de DLL dans le dossier Resolume — ca peut casser Spout integre.
