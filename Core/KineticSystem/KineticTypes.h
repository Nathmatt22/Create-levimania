// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/KineticSystem/KineticTypes.h
//
// Description des types de composants cinematiques et de leurs proprietes
// physiques (rayon, impact sur le Stress, capacite, role).
#pragma once

#include <cstdint>
#include <string_view>

namespace cl::kinetic {

// Famille fonctionnelle d'un composant.
enum class ComponentRole : std::uint8_t {
    Source,   // Genere de la rotation (moteur creatif, roue hydraulique, manivelle)
    Relay,    // Transmet la rotation (arbre, engrenage, courroie)
    Consumer, // Consomme de la rotation et fournit un travail (presse, melangeur, ventilateur)
    Storage,  // Accumule de l'energie cinetique (volant d'inertie)
};

// Type concret d'un composant, aligne sur les identifiants "create:".
enum class KineticType : std::uint8_t {
    Shaft,          // Axe de transmission
    Cogwheel,       // Petit engrenage
    LargeCogwheel,  // Grand engrenage (2:1)
    Gearbox,        // Boite de vitesses (relais multi-faces)
    BeltPulley,     // Poulie de courroie
    CreativeMotor,  // Moteur creatif (source reglable)
    WaterWheel,     // Roue hydraulique (source)
    LargeWaterWheel,// Grande roue hydraulique (source, plus lente/puissante)
    HandCrank,      // Manivelle (source manuelle temporaire)
    MechanicalPress,// Presse mecanique (consommatrice)
    MechanicalMixer,// Melangeur mecanique (consommatrice)
    EncasedFan,     // Ventilateur (consommatrice)
    Flywheel,       // Volant d'inertie (stockage)
    Unknown,
};

// Proprietes physiques statiques d'un type de composant.
struct ComponentProperties {
    ComponentRole role = ComponentRole::Relay;

    // Rayon relatif, en demi-blocs. Un petit engrenage vaut 1, un grand 2.
    // Sert au calcul des rapports d'engrenage.
    double radius = 1.0;

    // Impact sur le Stress, en SU par RPM (comme Create). Un consommateur
    // typique a une valeur non nulle, un relais pur a 0.
    double stressImpact = 0.0;

    // Capacite fournie, en SU. Non nul uniquement pour les sources.
    double stressCapacity = 0.0;

    // Vitesse de rotation nominale produite (pour les sources), en RPM.
    // 0 pour les composants qui ne generent pas.
    double nominalSpeed = 0.0;

    // Peut-on modifier la vitesse a l'interface (moteur creatif) ?
    bool configurableSpeed = false;

    std::string_view name = "unknown";
};

// Table de correspondance type -> proprietes (valeurs alignees sur Create
// autant que possible ; les valeurs exactes sont ajustables dans un seul
// endroit pour faciliter l'equilibrage).
ComponentProperties const& propertiesOf(KineticType type);

} // namespace cl::kinetic
