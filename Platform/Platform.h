// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Platform.h
//
// Interface de plateforme : tout ce que le noyau a besoin de demander au
// "monde" reel (lire un bloc, jouer un son, afficher une info-bulle). Le noyau
// ne connait que cette interface ; les implementations Windows (LeviLamina,
// ll::) et Android (LeviLauncher, pl::) la fournissent.
//
// L'interface reste volontairement minimale et sans dependance a Minecraft :
// les implementions traduisent les identifiants de blocs <-> KineticType.
#pragma once

#include "../Core/KineticSystem/KineticTypes.h"
#include "../Core/Rotation/Types.h"

#include <optional>
#include <string>

namespace cl::platform {

// Identification d'un bloc, telle que vue par le noyau (namespace + nom).
struct BlockId {
    std::string name; // ex. "create:shaft"
};

// Un bloc lu depuis le monde.
struct BlockInfo {
    BlockId id;
    int state = 0; // etat/rotation du bloc (encodage defini par la plateforme)
};

// Interface que chaque plateforme implemente.
class IPlatform {
public:
    virtual ~IPlatform() = default;

    // Lecture / ecriture du monde.
    virtual std::optional<BlockInfo> getBlock(kinetic::BlockPos pos) const = 0;
    virtual void setBlock(kinetic::BlockPos pos, BlockId id, int state) = 0;

    // Retour utilisateur.
    virtual void playSound(std::string const& sound) = 0;
    virtual void showActionBar(std::string const& text) = 0;

    // Journalisation.
    virtual void log(std::string const& message) = 0;
};

// --- Correspondance identifiant <-> type cinetique -------------------------

// Convertit "create:shaft" en KineticType. Renvoie nullopt si le bloc n'est pas
// un composant cinematique connu.
std::optional<kinetic::KineticType> typeFromBlockId(std::string const& id);

// Convertit un type cinetique en identifiant de bloc ("create:shaft").
std::string blockIdFromType(kinetic::KineticType type);

} // namespace cl::platform
