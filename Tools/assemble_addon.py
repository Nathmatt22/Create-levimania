#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# CreateLeviLamina - Tools/assemble_addon.py
#
# Assemble l'addon data-driven (BP + RP) du projet CreateLeviLamina en
# reutilisant les ressources originales du mod Create Amethyst (textures,
# modeles, sons, langue). Aucune ressource originale n'est modifiee : les
# fichiers sont copies tels quels dans Resources/, puis packages.
#
# Usage:
#   python3 Tools/assemble_addon.py --src /tmp/opencode/create_src \
#                                   --out /root/CreateLeviLamina/Resources
#
# L'addon produit est ensuite copie dans l'instance LeviLauncher par
# l'utilisateur (ou via le script d'installation).

import argparse
import json
import os
import shutil
import sys

# Ressources de la phase 1 (moteur creatif, arbres, engrenages, boite).
PHASE1_BLOCK_MODELS = [
    "creative_motor_block",
    "creative_motor_block_vertical",
    "creative_motor_item",
    "shaft_block",
    "shaft_item",
    "cogwheel_block",
    "cogwheel_item",
    "large_cogwheel",
    "large_cogwheel_item",
    "gearbox",
    "gearbox_item",
    "andesite_encased_shaft",
    "andesite_encased_shaft_item",
    "andesite_encased_cogwheel",
    "andesite_encased_cogwheel_bottom",
    "andesite_encased_cogwheel_top",
    "andesite_encased_cogwheel_top_bottom",
    "andesite_encased_cogwheel_item",
    "andesite_encased_large_cogwheel",
    "andesite_encased_large_cogwheel_bottom",
    "andesite_encased_large_cogwheel_top",
    "andesite_encased_large_cogwheel_top_bottom",
    "brass_encased_shaft",
    "brass_encased_shaft_item",
    "brass_encased_cogwheel",
    "brass_encased_large_cogwheel",
]

PHASE1_BLOCK_TEXTURES = [
    "creative_motor",
    "shaft",
    "cogwheel",
    "large_cogwheel",
    "gearbox",
    "gearbox_top",
    "cogwheel_axis",
    "andesite_casing",
    "brass_casing",
    "brass_gearbox",
]

PHASE1_ITEM_TEXTURES = [
    "andesite_alloy",
    "brass_ingot",
    "zinc_ingot",
    "electron_tube",
]


def copy(src_root, rel_src, out_root, rel_dst):
    s = os.path.join(src_root, rel_src)
    if not os.path.exists(s):
        print(f"  [skip] manquant: {rel_src}")
        return False
    d = os.path.join(out_root, rel_dst)
    os.makedirs(os.path.dirname(d), exist_ok=True)
    shutil.copy2(s, d)
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", required=True, help="Dossier de l'archive Create Amethyst extraite")
    ap.add_argument("--out", required=True, help="Dossier Resources/ de CreateLeviLamina")
    args = ap.parse_args()

    src = args.src
    out = args.out
    rp_src = os.path.join(src, "resource_packs", "main_rp")
    rp_dst = os.path.join(out, "addon", "CreateLeviLamina_RP")
    bp_dst = os.path.join(out, "addon", "CreateLeviLamina_BP")

    print("== Copie des modeles de blocs (phase 1) ==")
    for name in PHASE1_BLOCK_MODELS:
        copy(src, f"resource_packs/main_rp/models/blocks/{name}.geo.json",
             out, f"addon/CreateLeviLamina_RP/models/blocks/{name}.geo.json")

    print("== Copie des textures de blocs ==")
    for name in PHASE1_BLOCK_TEXTURES:
        copy(src, f"resource_packs/main_rp/textures/blocks/create/{name}.png",
             out, f"addon/CreateLeviLamina_RP/textures/blocks/create/{name}.png")

    print("== Copie des textures d'items ==")
    for name in PHASE1_ITEM_TEXTURES:
        copy(src, f"resource_packs/main_rp/textures/items/create/{name}.png",
             out, f"addon/CreateLeviLamina_RP/textures/items/create/{name}.png")

    print("== Copie de la langue anglaise (extrait) ==")
    os.makedirs(os.path.join(rp_dst, "texts"), exist_ok=True)
    lang_src = os.path.join(rp_src, "texts", "en_US.lang")
    if os.path.exists(lang_src):
        keep_prefixes = (
            "tile.create.creative_motor", "tile.create.shaft", "tile.create.cogwheel",
            "tile.create.large_cogwheel", "tile.create.gearbox", "entity.create.",
            "item.create.", "create.", "block.create.",
        )
        with open(lang_src, encoding="utf-8") as f:
            lines = f.readlines()
        with open(os.path.join(rp_dst, "texts", "en_US.lang"), "w", encoding="utf-8") as f:
            f.write("## Extrait de la traduction Create (phase 1) - CreateLeviLamina\n")
            for ln in lines:
                if ln.startswith(keep_prefixes):
                    f.write(ln)
        print(f"  [{len(lines)} lignes lues, extrait ecrit]")

    print("\nTermine. L'addon se trouve dans:", os.path.join(out, "addon"))
    print("Les manifestes et blocs sont generes par Tools/generate_data.py.")


if __name__ == "__main__":
    main()
