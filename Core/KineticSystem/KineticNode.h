// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/KineticSystem/KineticNode.h
//
// Un noeud represente un composant cinematique place dans le monde.
// Il ne contient que l'etat logique : le rendu et les donnees specifiques a
// Minecraft sont geres par la couche plateforme.
#pragma once

#include "KineticTypes.h"
#include "../Rotation/Types.h"

#include <cstdint>
#include <optional>
#include <string>

namespace cl::kinetic {

// Etat resolu d'un noeud apres passage du solveur.
struct KineticState {
    bool powered = false;     // Un source atteint ce noeud.
    bool stalled = false;     // Reseau bloque (conflit de vitesse / surcharge / axe incoherent).
    double speed = 0.0;       // Vitesse en RPM (valeur absolue).
    int direction = 0;        // +1 ou -1 (sens de rotation), 0 si arrete.
};

class KineticNode {
public:
    KineticNode() = default;
    KineticNode(BlockPos pos, KineticType type, Axis axis)
        : mPos(pos), mType(type), mAxis(axis) {}

    BlockPos pos() const { return mPos; }
    KineticType type() const { return mType; }
    Axis axis() const { return mAxis; }
    void setAxis(Axis a) { mAxis = a; }

    ComponentProperties const& properties() const { return propertiesOf(mType); }
    ComponentRole role() const { return properties().role; }

    // --- Parametres configurables (moteur creatif) --------------------------

    // Vitesse demandee pour une source, en RPM. Ignoree pour les non-sources.
    double requestedSpeed() const { return mRequestedSpeed; }
    void setRequestedSpeed(double rpm) { mRequestedSpeed = clampSpeed(rpm); }

    // Plage de vitesse acceptee, identique a Create : -256..256, multiples de 8
    // en pratique mais toutes valeurs sont tolerantes.
    static double clampSpeed(double rpm) {
        if (rpm > 256.0) return 256.0;
        if (rpm < -256.0) return -256.0;
        return rpm;
    }

    // --- Etat resolu (ecrit par le solveur) ---------------------------------

    KineticState const& state() const { return mState; }
    void setState(KineticState s) { mState = s; }

    // Identifiant unique stable (utile pour le reseau / logs).
    std::uint64_t id() const { return mId; }
    void setId(std::uint64_t id) { mId = id; }

    // Version incrementee a chaque changement structurel ; permet au solveur
    // de ne recalculer que les reseaux modifies.
    std::uint32_t revision() const { return mRevision; }
    void touch() { ++mRevision; }

private:
    BlockPos mPos;
    KineticType mType = KineticType::Unknown;
    Axis mAxis = Axis::X;
    double mRequestedSpeed = 0.0;
    KineticState mState;
    std::uint64_t mId = 0;
    std::uint32_t mRevision = 0;
};

} // namespace cl::kinetic
