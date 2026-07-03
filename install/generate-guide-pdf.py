#!/usr/bin/env python3
"""
Genere le guide PDF debutant 3DS2SPOUT.
Usage: python install/generate-guide-pdf.py
"""
from __future__ import annotations

import os
import sys
from pathlib import Path

try:
    from fpdf import FPDF
except ImportError:
    print("Installation de fpdf2...")
    os.system(f"{sys.executable} -m pip install fpdf2 --quiet")
    from fpdf import FPDF

# --- Palette: bleu turquoise sombre desature ---
C_BG = (26, 40, 48)          # #1a2830
C_BG2 = (34, 52, 62)         # #22343e
C_ACCENT = (74, 138, 148)    # #4a8a94
C_ACCENT2 = (92, 154, 163)    # #5c9aa3
C_TEXT = (232, 238, 240)     # #e8eef0
C_MUTED = (139, 170, 180)    # #8baab4
C_WHITE = (255, 255, 255)

SCRIPT_DIR = Path(__file__).resolve().parent
OUT_DIR = SCRIPT_DIR / "guide"
OUT_PDF = OUT_DIR / "GUIDE-DEBUTANT.pdf"

FONT_CANDIDATES = [
    Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts" / "segoeui.ttf",
    Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts" / "arial.ttf",
    Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
    Path("/System/Library/Fonts/Supplemental/Arial.ttf"),
]


def find_font() -> Path | None:
    for p in FONT_CANDIDATES:
        if p.is_file():
            return p
    return None


class GuidePDF(FPDF):
    def __init__(self) -> None:
        super().__init__(orientation="P", unit="mm", format="A4")
        self.set_auto_page_break(auto=True, margin=22)
        self.font_name = "GuideFont"
        font = find_font()
        if font:
            self.add_font(self.font_name, "", str(font))
            self.add_font(self.font_name, "B", str(font))
        else:
            self.font_name = "Helvetica"

    def _set_font(self, style: str = "", size: int = 11) -> None:
        self.set_font(self.font_name, style, size)

    def header(self) -> None:
        if self.page_no() == 1:
            return
        self.set_fill_color(*C_BG2)
        self.rect(0, 0, 210, 12, "F")
        self.set_text_color(*C_MUTED)
        self._set_font("", 8)
        self.set_xy(15, 4)
        self.cell(0, 5, "3DS2SPOUT — Guide débutant", align="L")
        self.set_xy(-45, 4)
        self.cell(30, 5, f"Page {self.page_no()}", align="R")
        self.ln(14)

    def footer(self) -> None:
        self.set_y(-14)
        self.set_draw_color(*C_ACCENT)
        self.line(15, self.get_y(), 195, self.get_y())
        self.set_text_color(*C_MUTED)
        self._set_font("", 7)
        self.cell(0, 8, "MIT License — github.com (3DS2SPOUT)", align="C")

    def cover(self) -> None:
        self.add_page()
        self.set_fill_color(*C_BG)
        self.rect(0, 0, 210, 297, "F")
        # bande accent
        self.set_fill_color(*C_ACCENT)
        self.rect(0, 88, 210, 3, "F")
        self.set_text_color(*C_WHITE)
        self._set_font("B", 32)
        self.set_xy(20, 55)
        self.cell(0, 14, "3DS2SPOUT")
        self.set_text_color(*C_ACCENT2)
        self._set_font("", 14)
        self.set_xy(20, 72)
        self.cell(0, 8, "Ta 3DS en caméra sans fil — source Spout")
        self.set_text_color(*C_MUTED)
        self._set_font("", 11)
        self.set_xy(20, 100)
        self.multi_cell(
            170,
            6,
            "Guide pas à pas pour débutants\n"
            "4 étapes — aucune compilation, aucun fichier à éditer",
        )
        self.set_text_color(*C_ACCENT2)
        self._set_font("B", 10)
        self.set_xy(20, 250)
        self.cell(0, 6, "Version 1.1 — 2026")

    def section_title(self, num: str, title: str) -> None:
        self.add_page()
        self.set_fill_color(*C_BG)
        self.rect(0, 0, 210, 297, "F")
        self.set_fill_color(*C_BG2)
        self.rect(15, 18, 180, 22, "F")
        self.set_text_color(*C_ACCENT)
        self._set_font("B", 10)
        self.set_xy(20, 22)
        self.cell(0, 6, f"ETAPE {num}")
        self.set_text_color(*C_WHITE)
        self._set_font("B", 16)
        self.set_xy(20, 30)
        self.cell(0, 8, title)
        self.set_y(50)

    def body_text(self, text: str) -> None:
        self.set_text_color(*C_TEXT)
        self._set_font("", 10)
        self.set_x(20)
        self.multi_cell(170, 5.5, text)
        self.ln(3)

    def bullet(self, text: str) -> None:
        self.set_text_color(*C_ACCENT2)
        self._set_font("B", 10)
        self.set_x(22)
        self.cell(5, 5.5, chr(8226))
        self.set_text_color(*C_TEXT)
        self._set_font("", 10)
        self.multi_cell(163, 5.5, text)
        self.ln(1)

    def note_box(self, text: str) -> None:
        y = self.get_y() + 2
        self.set_fill_color(*C_BG2)
        self.set_draw_color(*C_ACCENT)
        self.rect(20, y, 170, 18, "DF")
        self.set_text_color(*C_MUTED)
        self._set_font("", 9)
        self.set_xy(24, y + 4)
        self.multi_cell(162, 4.5, text)
        self.set_y(y + 22)

    def table_header(self, cols: list[tuple[str, int]]) -> None:
        self.set_fill_color(*C_ACCENT)
        self.set_text_color(*C_WHITE)
        self._set_font("B", 9)
        x = 20
        for label, w in cols:
            self.set_xy(x, self.get_y())
            self.cell(w, 7, label, border=0, fill=True)
            x += w
        self.ln(8)

    def table_row(self, cols: list[tuple[str, int]]) -> None:
        self.set_fill_color(*C_BG2)
        self.set_text_color(*C_TEXT)
        self._set_font("", 9)
        x = 20
        y = self.get_y()
        for label, w in cols:
            self.set_xy(x, y)
            self.cell(w, 6, label, border=0, fill=True)
            x += w
        self.ln(7)


def build() -> None:
    pdf = GuidePDF()
    pdf.cover()

    # Etape 0 - ce qu'il te faut
    pdf.section_title("0", "Ce qu'il te faut")
    pdf.body_text(
        "Avant de commencer, vérifie que tu as ces 5 choses. "
        "Tout le reste s'installe automatiquement à l'étape 1."
    )
    pdf.ln(2)
    pdf.table_header([("Élément", 75), ("Détail", 95)])
    rows = [
        ("PC Windows 10 ou 11", "Connecté au même WiFi que la 3DS"),
        ("Un logiciel compatible Spout", "Resolume, TouchDesigner, OBS…"),
        ("Python 3.10+", "python.org — coche « Add to PATH »"),
        ("Nintendo 3DS modée", "Luma3DS + Homebrew Launcher"),
        ("La carte SD de la 3DS", "Pour y copier l'application (1 fois)"),
    ]
    for r in rows:
        pdf.table_row([(r[0], 75), (r[1], 95)])
    pdf.ln(4)
    pdf.note_box(
        "3DS pas encore modée ? Suis le guide officiel 3ds.hacks.guide (une seule fois),\n"
        "puis reviens ici."
    )

    # Etape 1 - PC auto
    pdf.section_title("1", "PC — installation automatique")
    pdf.bullet("Télécharge le projet (zip GitHub Releases) et dézippe-le")
    pdf.bullet("Double-clique sur install\\INSTALL.cmd")
    pdf.bullet("C'est tout : packages Python et Spout s'installent tout seuls")
    pdf.ln(2)
    pdf.note_box(
        "Si Windows demande d'autoriser le réseau au premier lancement du bridge :\n"
        "accepte (le flux vidéo passe par le WiFi, port UDP 5000)."
    )

    # Etape 2 - SD
    pdf.section_title("2", "3DS — copier l'application (1 fois)")
    pdf.bullet("Éteins la 3DS et mets sa carte SD dans le PC")
    pdf.bullet("Copie 3ds-cam-stream.3dsx dans le dossier « 3ds » de la carte SD")
    pdf.bullet("Remets la carte SD dans la 3DS et rallume-la")
    pdf.ln(2)
    pdf.note_box(
        "C'est la seule fois où tu touches à la carte SD.\n"
        "Toute la configuration se fait ensuite sur l'écran tactile de la 3DS."
    )

    # Etape 3 - connexion
    pdf.section_title("3", "Connecter la 3DS au PC")
    pdf.bullet("PC : double-clique sur install\\run-bridge.cmd")
    pdf.bullet("Lis la ligne « IP PC pour la 3DS : 192.168.x.x » — note ce numéro")
    pdf.bullet("3DS : Homebrew Launcher -> 3DS2SPOUT")
    pdf.bullet("Touche le bouton IP PC (en bas à gauche) et tape le numéro -> OK")
    pdf.ln(2)
    pdf.note_box(
        "L'IP est enregistrée sur la 3DS : à refaire seulement si l'IP du PC change.\n"
        "Astuce : le bouton en haut à droite de cet écran passe l'interface en anglais."
    )

    # Etape 4 - premier stream
    pdf.section_title("4", "Premier stream")
    pdf.bullet("Dans ton logiciel visuel : ajoute la source Spout « 3DS2SPOUT »")
    pdf.bullet("3DS : appuie sur REC")
    pdf.bullet("L'écran affiche « PC OK » + les FPS : l'image arrive en direct")
    pdf.ln(4)
    pdf.set_fill_color(*C_ACCENT)
    pdf.rect(20, pdf.get_y(), 170, 14, "F")
    pdf.set_text_color(*C_WHITE)
    pdf._set_font("B", 11)
    pdf.set_xy(20, pdf.get_y() + 4)
    pdf.cell(170, 6, "C'est prêt — amuse-toi bien !", align="C")

    # Controles
    pdf.section_title("5", "Contrôles 3DS")
    pdf.table_header([("Bouton", 55), ("Action", 115)])
    pdf.table_row([("REC", 55), ("Démarre / arrête le stream", 115)])
    pdf.table_row([("EXTERNE / INTERNE", 55), ("Caméra arrière / avant", 115)])
    pdf.table_row([("FLUIDE / HD", 55), ("Qualité 200p ou 400p", 115)])
    pdf.table_row([("IP PC", 55), ("IP du PC + langue FR/EN", 115)])
    pdf.table_row([("START", 55), ("Quitter", 115)])

    # Depannage
    pdf.section_title("6", "Ça ne marche pas ?")
    pdf.table_header([("Problème", 65), ("Solution", 105)])
    fixes = [
        ("Pas d'image dans le logiciel", "REC activé ? IP PC bien saisie ? Même WiFi ?"),
        ("La 3DS affiche ATTENTE PC", "Relance run-bridge.cmd et autorise le pare-feu"),
        ("Erreur Spout au démarrage", "Installe Visual C++ Redistributable x64"),
        ("SpoutLibrary.dll manquante", "Relance install\\INSTALL.cmd"),
        ("Python introuvable", "Réinstalle-le en cochant « Add to PATH »"),
        ("Image saccadée", "Bouton FLUIDE sur la 3DS, rapproche-toi de la box"),
    ]
    for prob, sol in fixes:
        pdf.table_row([(prob, 65), (sol, 105)])
    pdf.ln(6)
    pdf.note_box(
        "Développeurs (compiler le .3dsx, déploiement WiFi, icône menu HOME) :\n"
        "voir README.md, scripts/README.md et docs/FORWARDER-FBI.md du dépôt."
    )

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pdf.output(str(OUT_PDF))
    print(f"PDF genere : {OUT_PDF}")


if __name__ == "__main__":
    build()
