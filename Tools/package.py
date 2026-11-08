#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# CreateLeviLamina - Tools/package.py
#
# Empaquette l'addon data-driven (BP + RP) et le manifeste de mod dans une
# archive .mcaddon-like (ZIP) installable, et un mod.json pour LeviLauncher.
#
# Usage:
#   python3 Tools/package.py --out /root/outbox
#
# Produit :
#   CreateLeviLamina_<version>.zip  (BP + RP a importer comme addon)
#   mod.json                        (metadonnees du mod natif)

import argparse
import os
import shutil
import sys
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "Resources")
ADDON = os.path.join(RES, "addon")
VERSION = "0.1.0"


def zip_dir(zf, src_dir, arc_prefix):
    for base, _dirs, files in os.walk(src_dir):
        for f in files:
            full = os.path.join(base, f)
            rel = os.path.relpath(full, src_dir)
            zf.write(full, os.path.join(arc_prefix, rel))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(ROOT, "dist"))
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)

    if not os.path.isdir(ADDON):
        sys.exit("Addon introuvable. Lancez d'abord Tools/assemble_addon.py "
                 "puis Tools/generate_data.py.")

    zip_path = os.path.join(args.out, f"CreateLeviLamina_{VERSION}.zip")
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        zip_dir(zf, os.path.join(ADDON, "CreateLeviLamina_BP"), "CreateLeviLamina_BP")
        zip_dir(zf, os.path.join(ADDON, "CreateLeviLamina_RP"), "CreateLeviLamina_RP")

    mod_json = os.path.join(RES, "mod.json")
    if os.path.exists(mod_json):
        shutil.copy2(mod_json, os.path.join(args.out, "mod.json"))

    print("Empaquete:", zip_path)
    print("Mod:", os.path.join(args.out, "mod.json"))


if __name__ == "__main__":
    main()
