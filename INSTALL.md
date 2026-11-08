# Guide d'installation — CreateLeviLamina

Ce guide explique **étape par étape** comment installer et utiliser
CreateLeviLamina sur **Windows** (LeviLauncher ou LeviLamina classique) et sur
**Android** (LeviLauncher).

> ⚠️ **Phase 1 — prototype.** Le noyau cinétique, l'addon data-driven (blocs) et
> l'intégration LeviLamina sont fonctionnels. Les animations et les machines
> (presse, mixeur, roue hydraulique…) arrivent en Phase 2.

---

## 1. Pré-requis

| Élément | Version minimale | Remarque |
|---|---|---|
| Minecraft Bedrock (Windows ou Android) | 1.21.0+ | `min_engine_version` de l'addon |
| LeviLamina / LeviLauncher | 26.60.4+ | version compilée par la CI |
| Espace disque | ~50 Mo | mod + addon + ressources |

Le mod comporte **deux parties complémentaires**, à installer toutes les deux :

1. **L'addon** (`.zip` contenant `*_BP` et `*_RP`) → fournit les **blocs** et les
   **ressources** (modèles, textures, sons, langue).
2. **Le mod natif** (`.dll` Windows / `.so` Android) → fournit la **logique
   cinétique** (arbres, engrenages, transmission d'énergie, stress).

---

## 2. Installation sur Windows (LeviLauncher)

### 2.1 Installer LeviLauncher

1. Télécharge **LeviLauncher** depuis le dépôt officiel LiteLDev :
   <https://github.com/LiteLDev/LeviLauncher/releases>
2. Installe-le et lance-le.
3. Dans LeviLauncher, installe **LeviLamina** sur ton instance Minecraft Bedrock
   (bouton *Install LeviLamina* / *Inject*). Le launcher récupère automatiquement
   la version compatible avec ton build du jeu.

### 2.2 Récupérer les fichiers

**Option A — depuis les releases GitHub (recommandé)**

1. Va sur <https://github.com/Nathmatt22/Create-levimania/releases>.
2. Télécharge, pour la version voulue :
   - `CreateLeviLamina-<version>-windows-x64.zip` (le mod natif), et
   - `CreateLeviLamina-addon-<version>.zip` (l'addon BP + RP).

**Option B — depuis les artefacts CI**

1. Onglet **Actions**, ouvre le dernier run **build** vert.
2. En bas de page, section **Artifacts**, télécharge
   `Create-levimania-<sha>-client-windows-x64` (le `.dll`).
3. Dans le run **addon**, télécharge `CreateLeviLamina-addon` (le `.zip` addon).

**Option C — compiler soi-même** (voir §4).

### 2.3 Installer le mod natif

1. Ferme Minecraft et LeviLauncher.
2. Ouvre le dossier de l'instance. Selon ton installation :
   - **LeviLauncher** : `…/LeviLauncher/Minecraft/`
   - **LeviLamina manuel** : le dossier où se trouve `bedrock_server.exe` /
     l'exécutable du jeu.
3. Crée (si absent) le dossier `mods/`.
4. Place le fichier **`.dll`** dedans. Tu dois avoir, par exemple :

   ```
   Minecraft/
   ├── mods/
   │   └── CreateLeviLamina.dll
   ├── behavior_packs/
   ├── resource_packs/
   └── ...
   ```

5. *(Optionnel)* crée `mods/CreateLeviLamina/` avec un `config.json` si tu veux
   surcharger la configuration par défaut (voir §5).

### 2.4 Installer l'addon (blocs et ressources)

1. Ouvre le `.zip` de l'addon : il contient **deux dossiers** :
   `CreateLeviLamina_BP/` (comportements) et `CreateLeviLamina_RP/` (ressources).
2. Copie ces deux dossiers dans l'instance :
   - `CreateLeviLamina_BP` → `Minecraft/behavior_packs/`
   - `CreateLeviLamina_RP` → `Minecraft/resource_packs/`

   ```
   Minecraft/
   ├── behavior_packs/
   │   └── CreateLeviLamina_BP/
   └── resource_packs/
       └── CreateLeviLamina_RP/
   ```

3. Sur un **monde existant**, active-les manuellement : *Modifier le monde* →
   *Packs de comportement* → active **CreateLeviLamina BP**, puis *Packs de
   ressources* → active **CreateLeviLamina RP**.
4. *(Nouveau monde)* active-les à la création.

### 2.5 Lancer et vérifier

1. Lance l'instance via LeviLauncher.
2. Le journal doit afficher :
   ```
   CreateLeviLamina : chargement du noyau cinematique...
   CreateLeviLamina active.
   ```
3. En jeu, ouvre l'inventaire créatif : un onglet **Create** doit apparaître avec
   `creative_motor`, `shaft`, `cogwheel`, `large_cogwheel`, `gearbox`.
4. Pose un **moteur créatif**, puis des **arbres** et des **engrenages** collés :
   le réseau se construit automatiquement.

---

## 3. Installation sur Android (LeviLauncher)

1. Installe **LeviLauncher** sur Android :
   <https://github.com/LiteLDev/LeviLauncher/releases> (APK).
2. Dans l'app, installe **LeviLamina** puis Minecraft Bedrock 1.21+.
3. Accède au stockage de l'instance (via l'app ou un gestionnaire de fichiers) :
   - `mods/` pour le **`.so`** natif Android,
   - `behavior_packs/` / `resource_packs/` pour l'**addon**.
4. Copie le `.so` et les dossiers BP/RP comme au §2.4, puis active les packs dans
   le monde.

> La compilation Android utilise le NDK ; voir `Platform/Android/` et le guide
> Android officiel de LeviLamina.

---

## 4. Construire depuis les sources

### 4.1 Tests du noyau (aucun SDK requis)

```bash
python3 Tools/run_tests.py
```

### 4.2 Mod natif Windows

Pré-requis : [xmake](https://xmake.io/) + **Visual Studio 2022** (ou LLVM/clang-cl).

```bash
xmake repo -u
xmake f -a x64 -m release -p windows --target_type=client -y
xmake -v -y
```

Le `.dll` est produit dans `bin/`. Copie-le dans `mods/`.

### 4.3 Regénérer l'addon

```bash
python3 Tools/generate_data.py --out Resources
python3 Tools/package.py        --out dist
```

Le `.zip` installable est produit dans `dist/`.

---

## 5. Configuration

Un fichier `config.json` peut être placé dans `mods/CreateLeviLamina/`. Valeurs
par défaut (alignées sur Create d'origine) :

| Clé | Défaut | Description |
|---|---|---|
| `creativeMotorCapacity` | `16384` | Capacité du moteur créatif (SU) |
| `creativeMotorSpeed` | `256` | Vitesse nominale (RPM) |
| `maxBeltLength` | `20` | Longueur maximale d'une courroie |
| `enableNetworkDebug` | `false` | Journalisation détaillée des réseaux |

Exemple :

```json
{
  "creativeMotorCapacity": 16384,
  "creativeMotorSpeed": 256,
  "maxBeltLength": 20,
  "enableNetworkDebug": false
}
```

---

## 6. Dépannage

| Symptôme | Cause probable | Solution |
|---|---|---|
| Onglet Create absent | Addon non activé | Active BP **et** RP dans le monde |
| Blocs posés mais immobiles | Mod natif absent/chargé | Vérifie le `.dll`/`.so` dans `mods/`, regarde le log |
| `CreateLeviLamina active` absent du log | Version LeviLamina incompatible | Mets à jour LeviLamina (26.60.4+) |
| Erreur de chargement du pack | Mauvais dossier | Le contenu de BP/RP doit être à la racine du dossier copié |

---

## 7. Désinstallation

1. Supprime `mods/CreateLeviLamina.dll` (ou `.so`).
2. Supprime `behavior_packs/CreateLeviLamina_BP/` et
   `resource_packs/CreateLeviLamina_RP/`.
3. Désactive les packs dans les mondes qui les utilisaient.

---

## Liens utiles

- Dépôt : <https://github.com/Nathmatt22/Create-levimania>
- LeviLamina : <https://github.com/LiteLDev/LeviLamina>
- LeviLauncher : <https://github.com/LiteLDev/LeviLauncher>
- Audit technique : [`Documentation/AUDIT.md`](Documentation/AUDIT.md)
