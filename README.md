# CreateLeviLamina

Portage progressif du mod **Create** (à l'origine pour le chargeur *Amethyst*) vers
**LeviLamina / LeviLauncher**, pour **Windows** et **Android**.

Le projet réutilise les ressources graphiques/sonores/linguistiques originales de
Create et réimplémente toute la logique native, qui était indissociable d'Amethyst.

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

- **Windows** : `xmake f --target_type=client && xmake` (SDK LeviLamina requis).
  Copier le `.dll` dans `mods/CreateLeviLamina/`.
- **Android** : via le NDK, voir `Platform/Android/` et le guide Android de
  LeviLamina.

## Documentation

- `Documentation/AUDIT.md` — audit complet de l'archive source et stratégie.
