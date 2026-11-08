# Changelog

Toutes les modifications notables de ce projet sont documentées dans ce fichier.

Le format suit [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) et ce
projet adhère au [versionnage sémantique](https://semver.org/lang/fr/).

## [0.1.0] - 2026-11-08

Première version — Phase 1 (prototype).

### Ajouté

- **Noyau cinétique portable** (C++20 pur, sans dépendance à Minecraft) :
  - types géométriques (`BlockPos`, `Axis`) ;
  - graphe cinétique et solveur de réseaux (arbres, engrenages, courroies,
    chaînes, transmission de couple, stress, surcharge/conflit).
- **Couche monde** (`KineticWorld`) : placement/retrait de blocs et connexions
  automatiques entre composants.
- **Pont plateforme** (`KineticBridge`) : les événements Minecraft pilotent le
  noyau cinétique.
- **Adaptateurs** Windows (`ll::` + xmake) et Android (`pl::` + NDK).
- **Addon data-driven** réutilisant les modèles, textures, sons, langues et
  recettes de Create : blocs `creative_motor`, `shaft`, `cogwheel`,
  `large_cogwheel`, `gearbox` et variantes encased.
- **Outils** : assemblage, génération de données, packaging et runner de tests.
- **CI** : tests du noyau (Ubuntu), build Windows contre le SDK LeviLamina,
  empaquetage de l'addon.
- **Documentation** : `README.md`, `INSTALL.md`, `Documentation/AUDIT.md`.

### Notes

- Compilé contre **LeviLamina 26.60.x** (`target_type=client`).
- Les animations et les machines (presse, mixeur, ventilateur, roue hydraulique)
  sont prévues pour la Phase 2.

[0.1.0]: https://github.com/Nathmatt22/Create-levimania/releases/tag/v0.1.0
