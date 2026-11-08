# Rapport technique — audit et portage de Create (Amethyst) vers LeviLamina

**Projet :** CreateLeviLamina
**Source analysée :** `Create@0.1.1.amethyst`
**Date :** 2026-11-08
**Cible :** LeviLamina / LeviLauncher — Windows (x64) et Android (arm64)

---

## 1. Nature de l'archive source

L'archive `Create@0.1.1.amethyst` est en réalité un **ZIP** (extension propre au
gestionnaire de mods Amethyst) contenant 3790 entrées. Les chemins internes
utilisent des backslashes Windows, ce qui casse l'extraction avec `unzip` : il
faut normaliser `\` → `/` (fait via Python dans `Tools/assemble_addon.py`).

### 1.1 Manifeste (`mod.json`)

| Champ | Valeur |
|---|---|
| name | Create |
| version | 0.1.1 |
| namespace | `create` |
| author | FrederoxDev |
| uuid | `83fda108-79f7-4f66-8092-41343fc32bd1` |
| dépendance | Amethyst `>= 2.2.5` (uuid `e56c3987-fd4a-4590-9923-a07fc4be7250`) |
| platform | `win-client` uniquement |
| game_builds | version `1.26.42`, guid `E441A62C-3AC0-B238-4C4C-44205044422E` |

### 1.2 Structure

```
mod.json                     manifeste Amethyst
behavior_packs/main_bp/      BP : 612 recettes, 8 entités (aucun blocks/items)
resource_packs/main_rp/      RP : modèles, textures, UI, sons, langue, ponder
win-client/Create.dll        logique native (22 Mo)
```

- **BP manifest** uuid `3b42e82a-bd0c-4b6e-b32b-7658d6ddd098`
- **RP manifest** uuid `e5aca142-d61b-4efe-ad29-376769c2fdcf`
- `min_engine_version` = `1.21.0` pour les deux.

Aucun dossier `blocks/` ni `items/` côté BP : **les blocs et items sont
enregistrés par la DLL** (data-driven à l'exécution), pas par des fichiers de
définition.

---

## 2. Analyse de la DLL (`win-client/Create.dll`)

| Propriété | Valeur |
|---|---|
| taille | 21 523 968 octets |
| md5 | `d5e56fa0957c8887ccc43257f98187e3` |
| format | PE32+ x64 (machine `0x8664`), 8 sections |
| exports | 2 seulement : `?equals@Tag@@UEBA_NAEBV1@@Z`, `Initialize` |
| imports | KERNEL32, MSVCP140, USER32, VCRUNTIME140(_1), ntdll, api-ms-win-* |

### 2.1 Technique d'intégration

La DLL fait du **hooking natif** via `safetyhook` (`InlineHook`, `VmtHook`) :

- **166 cibles de hooks** détectées ;
- **1771 identifiants** `create:` embarqués ;
- utilise `Amethyst::GetPlatform` et `Amethyst::CustomPacketInternal`.

**Aucune référence à LeviLamina/LeviLauncher** : la binaire ne peut pas être
chargé tel quel. Toute la logique d'intégration (hooks, enregistrement des
blocs, paquets réseau) doit être **réécrite** contre les API LeviLamina.

### 2.2 Conséquence architecturale

Le portage ne peut pas se faire par simple re-hébergement de la DLL. Il faut
une **réimplémentation**, séparée en deux couches :

1. **Noyau cinétique portable** (C++ standard pur) — logique physique,
   testable hors Minecraft.
2. **Adaptateurs plateforme** — Windows (`xmake` + `ll::`) et Android
   (`cmake`/NDK + `pl::`).

---

## 3. Ressources récupérables (réutilisables sans modification)

| Catégorie | Chemin source | Quantité |
|---|---|---|
| Modèles de blocs | `resource_packs/main_rp/models/blocks/` | 667 `.geo.json` |
| Textures de blocs | `textures/blocks/create/` | 544 |
| Textures d'entités | `textures/entity/` | 283 |
| Textures d'items | `textures/items/create/` | 103 |
| UI | `ui/` | 40 fichiers |
| Sons | `sounds/create/` | 38 `.ogg` |
| Fichiers « ponder » | `ponder/` | 49 |
| Langue | `texts/en_US.lang` | 3315 lignes |
| Recettes BP | `recipes/kinetics/` | 258 (+354 autres) |
| Entités BP | `entities/` | 8 |
| Divers | `terrain_texture.json` (789 tiles), `sound_definitions.json`, `flipbook_textures.json`, `animations/potato_projectile.animation.json`, `materials/entity.material`, `render_controllers/` (2) |

Les modèles clés de la phase 1 sont confirmés présents :
`geometry.create.creative_motor_block`, `shaft_block`, `cogwheel_block`,
`large_cogwheel`, `gearbox`, et toutes les variantes « encased ».

Ces ressources sont **directement réutilisables** en BP/RP Bedrock, car le
format `.geo.json` / `terrain_texture.json` / `.lang` est identique sous
LeviLamina (addons data-driven).

---

## 4. Fonctionnalités identifiables

D'après les recettes, identifiants et textures, le mod couvre la mécanique
kinétique complète de Create :

- **Génération de rotation** : moteur créatif (16384 SU), roue hydraulique
  (water wheel), moulin à vent ;
- **Transmission** : arbres (shaft), engrenages (cogwheel), grandes roues
  (large cogwheel), boîtes de vitesses (gearbox), courroies (belt),
  chaînes (chain), « encased » (andesite/brass) ;
- **Machines** : presse mécanique, mixeur, ventilateur, scie, foreuse, etc. ;
- **Contraptions** : entités `contraption`, `carriage`, `gantry`, `seat`,
  `stationary_contraption`, `potato_projectile` ;
- **Blocs logiques** : leviers analogiques, gearshifts, clutchs, latches.

Constantes cinétiques (alignées sur Create) : `capacity=16384 SU`,
`nominalSpeed=256 RPM`, `kBaseRotationSpeed=256.0`, `kSpeedEpsilon=1e-4`,
`kMaxBeltLength=20`.

---

## 5. Incompatibilités et difficultés

| Difficulté | Impact | Mitigation |
|---|---|---|
| DLL dépend d'Amethyst | Non chargeable | Réimplémentation |
| Hooks natifs sur build `1.26.42` | Signatures obsolètes | Utiliser `ll::memory` / signatures LeviLamina |
| Enregistrement de blocs par la DLL | Pas d'API blocs custom | **Blocs data-driven** (BP) + tag porté par le C++ |
| Contraptions = entités custom | Complexe à reproduire | Hors phase 1 |
| Rendu des engrenages animés | Pas de `animation_controllers` fournis | Animations à écrire (phase 2) |
| Android : pas de `cmake` sur l'hôte actuel | Bloque le build NDK | Installer cmake/NDK, ou script CI |

---

## 6. Stratégie de portage

### Phase 0 — Audit (terminé)
Analyse archive, DLL, ressources, API cibles, environnement de build.

### Phase 1 — Prototype jouable (en cours)
- **Noyau cinétique** C++ pur : types, nœuds, réseaux, graphe + solveur
  (propagation O(V+E), solve incrémental, détection surcharge/conflit).
- Tests unitaires du noyau (12 tests, 38 vérifications).
- Addon data-driven : blocs `creative_motor`, `shaft`, `cogwheel`,
  `large_cogwheel`, `gearbox` réutilisant les modèles/textures originaux.
- UI de vitesse (affichage RPM, surbrillance réseau).

### Phase 2 — Machines
Presse, mixeur, ventilateur, roue hydraulique ; animation des composants ;
courroies et boîtes de vitesses complètes.

### Phase 3 — Contraptions et polish
Entités contraption, ponder UI, sons, i18n.

---

## 7. Environnement de build

- Hôte : aarch64, `g++ 13.3.0`, `make`, `python3`, `node v22.23.4`, `npm`, `git`.
- **`cmake` absent** → prévoir installation pour la cible Android.
- `unzip`/`zip` disponibles.
- Noyau compilé et testé avec :
  `g++ -std=c++20 -O2 ... && /tmp/opencode/test_kinetic` → **0 échec**.

---

## 8. Conclusion

Le portage est **réalisable** mais nécessite une **réécriture de la logique
native** (la DLL est indissociable d'Amethyst). Les ressources graphiques,
sonores et linguistiques sont **entièrement réutilisables**. La décision
d'architecture (noyau portable + adaptateurs par plateforme) permet de valider
la physique indépendamment de Minecraft, et de couvrir Windows et Android avec
un socle commun.
