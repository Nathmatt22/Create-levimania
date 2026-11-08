#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# CreateLeviLamina - Tools/generate_data.py
#
# Genere le squelette data-driven de l'addon CreateLeviLamina :
#   - manifestes BP et RP (UUIDs stables, dependance croisee) ;
#   - blocs personnalises (creative_motor, shaft, cogwheel, large_cogwheel, gearbox)
#     relies aux geometries et textures originales copiees par assemble_addon.py ;
#   - terrain_texture.json regenerant les entrees utilisees.
# Ceci remplace, pour la phase 1, l'enregistrement des blocs qui etait fait par
# la DLL native dans le mod Amethyst d'origine.

import argparse
import json
import os
import uuid

NS = "create"
PACK_VERSION = [1, 0, 0]
MIN_ENGINE = [1, 21, 0]

# UUIDs derives de facon stable (namespace + nom) pour rester reproductible.
def stable_uuid(name):
    return str(uuid.uuid5(uuid.NAMESPACE_URL, "creatlevilamina:" + name))

# Phase 1 : blocs places + leurs geometries / textures / faces.
BLOCKS = [
    {
        "id": "creative_motor",
        "geometry": "geometry.create.creative_motor_block",
        "texture": "create:creative_motor",
        "material_instance": "creative_motor",
        "map_color": "#6f6a63",
    },
    {
        "id": "shaft",
        "geometry": "geometry.create.shaft_block",
        "texture": "create:shaft",
        "material_instance": "shaft",
        "map_color": "#8a8a8a",
    },
    {
        "id": "cogwheel",
        "geometry": "geometry.create.cogwheel_block",
        "texture": "create:cogwheel",
        "material_instance": "cogwheel",
        "map_color": "#9a7b4f",
    },
    {
        "id": "large_cogwheel",
        "geometry": "geometry.create.large_cogwheel",
        "texture": "create:large_cogwheel",
        "material_instance": "large_cogwheel",
        "map_color": "#9a7b4f",
    },
    {
        "id": "gearbox",
        "geometry": "geometry.create.gearbox",
        "texture": "create:gearbox",
        "material_instance": "gearbox",
        "map_color": "#6f6a63",
    },
]

TERRAIN_TEXTURE = {
    "create:creative_motor": "textures/blocks/create/creative_motor",
    "create:shaft": "textures/blocks/create/shaft",
    "create:cogwheel": "textures/blocks/create/cogwheel",
    "create:large_cogwheel": "textures/blocks/create/large_cogwheel",
    "create:gearbox": "textures/blocks/create/gearbox",
}


def write_json(path, obj):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2)
        f.write("\n")


def block_json(b):
    identifier = f"{NS}:{b['id']}"
    return {
        "format_version": "1.21.0",
        "minecraft:block": {
            "description": {
                "identifier": identifier,
                "menu_category": {"category": "construction", "group": "itemGroup.name.create"},
            },
            "components": {
                "minecraft:geometry": b["geometry"],
                "minecraft:material_instances": {
                    "*": {
                        "texture": b["texture"],
                        "render_method": "opaque",
                        "face_dimming": True,
                        "ambient_occlusion": True,
                    }
                },
                "minecraft:destructible_by_mining": {"seconds_to_destroy": 1.0},
                "minecraft:destructible_by_explosion": {"explosion_resistance": 6.0},
                "minecraft:map_color": b["map_color"],
                "minecraft:light_dampening": 0,
                # Marqueur : le coeur C++ lira ce tag pour rattacher un noeud cinetique.
                "tag:create:kinetic": {},
            },
        },
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True, help="Dossier Resources/ de CreateLeviLamina")
    args = ap.parse_args()
    out = args.out

    bp = os.path.join(out, "addon", "CreateLeviLamina_BP")
    rp = os.path.join(out, "addon", "CreateLeviLamina_RP")

    # --- Manifestes ---
    bp_header_uuid = stable_uuid("bp.header")
    bp_mod_uuid = stable_uuid("bp.data")
    rp_header_uuid = stable_uuid("rp.header")
    rp_res_uuid = stable_uuid("rp.resources")

    write_json(os.path.join(bp, "manifest.json"), {
        "format_version": 2,
        "header": {
            "name": "Create LeviLamina BP",
            "description": "Logique data-driven du portage Create (phase 1).",
            "uuid": bp_header_uuid,
            "version": PACK_VERSION,
            "min_engine_version": MIN_ENGINE,
        },
        "modules": [
            {"type": "data", "uuid": bp_mod_uuid, "version": PACK_VERSION},
        ],
        "dependencies": [
            {"uuid": rp_header_uuid, "version": PACK_VERSION},
        ],
    })

    write_json(os.path.join(rp, "manifest.json"), {
        "format_version": 2,
        "header": {
            "name": "Create LeviLamina RP",
            "description": "Ressources reutilisees du mod Create (phase 1).",
            "uuid": rp_header_uuid,
            "version": PACK_VERSION,
            "min_engine_version": MIN_ENGINE,
        },
        "modules": [
            {"type": "resources", "uuid": rp_res_uuid, "version": PACK_VERSION},
        ],
        "metadata": {"authors": ["FrederoxDev (assets originaux)", "CreateLeviLamina"]},
    })

    # --- Blocs BP ---
    for b in BLOCKS:
        write_json(os.path.join(bp, "blocks", f"{b['id']}.json"), block_json(b))

    # --- terrain_texture.json (RP) ---
    write_json(os.path.join(rp, "textures", "terrain_texture.json"), {
        "resource_pack_name": "CreateLeviLamina",
        "texture_name": "atlas.terrain",
        "padding": 8,
        "num_mip_levels": 4,
        "texture_data": {
            k: {"textures": v} for k, v in TERRAIN_TEXTURE.items()
        },
    })

    # --- item_group pour regrouper dans l'inventaire creatif ---
    write_json(os.path.join(bp, "item_catalog", "create_group.json"), {
        "format_version": "1.21.0",
        "minecraft:item_group": {
            "description": {
                "identifier": "create:kinetics",
                "group_icon": {"textures": ["textures/items/create/cogwheel"]},
            }
        },
    })

    print("Genere:")
    print("  BP manifest:", bp_header_uuid)
    print("  RP manifest:", rp_header_uuid)
    print("  blocs:", ", ".join(b["id"] for b in BLOCKS))


if __name__ == "__main__":
    main()
