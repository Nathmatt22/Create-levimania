# CreateLeviLamina

Portage progressif du mod **Create** (à l'origine pour le chargeur *Amethyst*) vers
**LeviLamina / LeviLauncher**, pour **Windows** et **Android**.

Le projet réutilise les ressources graphiques/sonores/linguistiques originales de
Create et réimplémente toute la logique native, qui était indissociable d'Amethyst.

## Installation

Guide détaillé (dépannage, configuration, Android) : **[INSTALL.md](INSTALL.md)**.

### Windows — LeviLauncher

1. **Installer LeviLauncher** depuis
   <https://github.com/LiteLDev/LeviLauncher/releases>, puis installer
   **LeviLamina** sur une instance Minecraft Bedrock 1.21+.
2. **Télécharger les deux fichiers** depuis les
   [releases](https://github.com/Nathmatt22/Create-levimania/releases) :
   - `CreateLeviLamina-*-windows-x64.zip` — le mod natif (logique cinétique) ;
   - `CreateLeviLamina-addon-*.zip` — l'addon BP + RP (blocs et ressources).
3. **Installer le mod natif** : copier le **`.dll`** dans le dossier `mods/` de
   l'instance Minecraft (à côté de `behavior_packs/`, `resource_packs/`).
4. **Installer l'addon** : copier `CreateLeviLamina_BP/` dans
   `behavior_packs/` et `CreateLeviLamina_RP/` dans `resource_packs/`.
5. **Activer les packs** dans le monde : *Modifier le monde* → *Packs de
   comportement* (BP) puis *Packs de ressources* (RP).
6. **Lancer** l'instance : le log doit afficher `CreateLeviLamina active.` et un
   onglet **Create** apparaît dans l'inventaire créatif.

### Android — LeviLauncher

Même principe : installer LeviLamina, puis copier le **`.so`** dans `mods/` et les
dossiers BP/RP dans `behavior_packs/` et `resource_packs/`.

### Depuis les sources

```bash
python3 Tools/run_tests.py                 # tests du noyau (sans SDK)
xmake f -a x64 -m release -p windows --target_type=client -y && xmake   # mod natif
python3 Tools/generate_data.py --out Resources && python3 Tools/package.py --out dist  # addon
```

## État actuel — Phase 1 (prototype)

- Noyau cinétique portable (C++20 pur), testé hors Minecraft.
- Couche monde : placement, connexions automatiques, résolution.
- Pont plateforme : les événements Minecraft pilotent le noyau.
- Addon data-driven réutilisant les modèles/textures/recettes de Create.
- Adaptateurs Windows (`ll::`) et Android (`pl::`).

## Architecture

```
Core/            Noyau portable, sans dépendance à Minecraft
  Rotation/      Types géométriques (BlockPos, Axis)
  KineticSystem/ Graphe cinétique + solveur de réseaux (Stress, rapports)
  World/         KineticWorld : position <-> nœud, connexions automatiques
Platform/        Interface IPlatform + adaptateurs
  KineticBridge  Relie le monde Minecraft au noyau
  Windows/       Mod LeviLamina (xmake, ll::)
  Android/       Mod natif (NDK, pl::)
Resources/addon  BP + RP data-driven (générés par Tools/)
Tools/           Scripts d'assemblage / génération / tests / packaging
```

## Construire et tester le noyau (sans Minecraft)

```bash
python3 Tools/run_tests.py          # compile + exécute les 3 suites de tests
```

ou avec cmake :

```bash
cmake -B build && cmake --build build && ctest --test-dir build
```

## Générer l'addon installable

```bash
python3 Tools/assemble_addon.py --src <archive Create extraite> --out Resources
python3 Tools/generate_data.py  --out Resources
python3 Tools/package.py        --out dist
```

## Construire le mod natif

- **Windows** : `xmake f -a x64 -m release -p windows --target_type=client -y && xmake`
  (SDK LeviLamina requis). Copier le `.dll` dans `mods/`.
- **Android** : via le NDK, voir `Platform/Android/` et le guide Android de
  LeviLamina.

## Documentation

- `INSTALL.md` — guide d'installation pas à pas (Windows & Android).
- `Documentation/AUDIT.md` — audit complet de l'archive source et stratégie.
