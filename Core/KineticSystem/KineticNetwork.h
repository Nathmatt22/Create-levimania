// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/KineticSystem/KineticNetwork.h
//
// Un reseau regroupe des composants cinematiques connectes entre eux.
// Il expose une vue agregee (vitesse, sens, capacite, charge) calculee par le
// solveur, et permet au reste du jeu d'interroger l'etat sans recalculer le
// graphe a chaque tick.
#pragma once

#include "KineticNode.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cl::kinetic {

using NodeId = std::uint64_t;

// Etat agrege d'un reseau.
struct NetworkState {
    bool valid = false;     // Un reseau valide a une source et une vitesse coherente.
    bool stalled = false;   // Conflit, surcharge ou axe incoherent.
    bool overstressed = false;
    double speed = 0.0;     // Vitesse commune, en RPM.
    int direction = 0;      // +1 / -1 / 0.
    double capacity = 0.0;  // Somme des capacites des sources (SU).
    double load = 0.0;      // Somme des impacts des consommateurs (SU x RPM / base).
    std::string stallReason;
};

class KineticNetwork {
public:
    explicit KineticNetwork(std::uint64_t id) : mId(id) {}

    std::uint64_t id() const { return mId; }

    void addNode(NodeId id) { mNodes.push_back(id); }
    std::vector<NodeId> const& nodes() const { return mNodes; }

    // Marque le reseau comme "sale" : le solveur le recalculera.
    void markDirty() { mState.valid = false; mDirty = true; }
    bool dirty() const { return mDirty; }
    void clearDirty() { mDirty = false; }

    NetworkState const& state() const { return mState; }
    void setState(NetworkState s) { mState = std::move(s); }

private:
    std::uint64_t mId;
    std::vector<NodeId> mNodes;
    NetworkState mState;
    bool mDirty = true;
};

} // namespace cl::kinetic
