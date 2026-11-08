// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/World/KineticWorld.cpp
#include "KineticWorld.h"

#include <array>

namespace cl::kinetic {

namespace {

// Les six directions cardinales, associees a leur axe et a leur signe.
struct Dir {
    Axis axis;
    int dx;
    int dy;
    int dz;
};

constexpr std::array<Dir, 6> kDirs = {{
    {Axis::X, 1, 0, 0},
    {Axis::X, -1, 0, 0},
    {Axis::Y, 0, 1, 0},
    {Axis::Y, 0, -1, 0},
    {Axis::Z, 0, 0, 1},
    {Axis::Z, 0, 0, -1},
}};

bool isGearLike(KineticType t) {
    return t == KineticType::Cogwheel || t == KineticType::LargeCogwheel;
}

bool isRotary(KineticType t) {
    return t != KineticType::Unknown;
}

// Un arbre / une poulie transmet uniquement le long de son axe. Un engrenage
// transmet par engrenage dans toutes les directions. Une boite de vitesses
// transmet sur toutes ses faces. Une source est couplee comme un arbre sur son
// axe, mais peut aussi entrainer directement un engrenage adjacent.
bool transmitsAlongAxis(KineticType t) {
    return t == KineticType::Shaft || t == KineticType::BeltPulley ||
           t == KineticType::CreativeMotor || t == KineticType::WaterWheel ||
           t == KineticType::LargeWaterWheel || t == KineticType::HandCrank;
}

} // namespace

bool KineticWorld::couple(Axis /*sharedAxis*/, KineticType a, KineticType b, Coupling& out) {
    if (!isRotary(a) || !isRotary(b)) {
        return false;
    }

    // Deux engrenages en prise : rapport de rayon + inversion de sens.
    if (isGearLike(a) && isGearLike(b)) {
        out = Coupling::Gear;
        return true;
    }

    // Boite de vitesses : couplage en arbre sur toutes les faces (simplifie).
    if (a == KineticType::Gearbox || b == KineticType::Gearbox) {
        out = Coupling::Shaft;
        return true;
    }

    // Un engrenage face a un arbre : engrenage (l'arbre entraine la roue).
    if (isGearLike(a) || isGearLike(b)) {
        out = Coupling::Gear;
        return true;
    }

    // Deux composants purement axiaux : ils ne se couplent que si l'axe partage
    // correspond a leur axe de rotation. Le noeud transmet le long de son axe.
    // sharedAxis est l'axe de la direction de voisinage.
    if (transmitsAlongAxis(a) || transmitsAlongAxis(b)) {
        out = Coupling::Shaft;
        return true;
    }

    out = Coupling::Shaft;
    return true;
}

NodeId KineticWorld::place(BlockPos pos, KineticType type, Axis axis) {
    if (auto existing = nodeAt(pos)) {
        // Remplace le type/axe : on retire d'abord l'ancien noeud.
        erase(pos);
    }
    NodeId id = mGraph.addNode(KineticNode(pos, type, axis));
    mNodesByPos[pos] = id;
    refreshConnections();
    return id;
}

bool KineticWorld::erase(BlockPos pos) {
    auto it = mNodesByPos.find(pos);
    if (it == mNodesByPos.end()) {
        return false;
    }
    mGraph.removeNode(it->second);
    mNodesByPos.erase(it);
    refreshConnections();
    return true;
}

bool KineticWorld::setAxis(BlockPos pos, Axis axis) {
    auto it = mNodesByPos.find(pos);
    if (it == mNodesByPos.end()) {
        return false;
    }
    if (auto* n = mGraph.node(it->second)) {
        n->setAxis(axis);
        mGraph.markNodeDirty(it->second);
    }
    refreshConnections();
    return true;
}

bool KineticWorld::setSpeed(BlockPos pos, double rpm) {
    auto it = mNodesByPos.find(pos);
    if (it == mNodesByPos.end()) {
        return false;
    }
    if (auto* n = mGraph.node(it->second)) {
        if (n->role() != ComponentRole::Source) {
            return false;
        }
        n->setRequestedSpeed(rpm);
        mGraph.markNodeDirty(it->second);
    }
    return true;
}

std::optional<NodeId> KineticWorld::nodeAt(BlockPos pos) const {
    auto it = mNodesByPos.find(pos);
    if (it == mNodesByPos.end()) {
        return std::nullopt;
    }
    return it->second;
}

void KineticWorld::connectTo(BlockPos pos) {
    auto selfIt = mNodesByPos.find(pos);
    if (selfIt == mNodesByPos.end()) {
        return;
    }
    auto const* self = mGraph.node(selfIt->second);
    if (!self) {
        return;
    }

    for (auto const& d : kDirs) {
        BlockPos nb = pos.offset(d.dx, d.dy, d.dz);
        auto nbIt = mNodesByPos.find(nb);
        if (nbIt == mNodesByPos.end()) {
            continue;
        }
        // Evite les doublons de connexions (a -> b puis b -> a).
        if (nbIt->second < selfIt->second) {
            continue;
        }
        auto const* other = mGraph.node(nbIt->second);
        if (!other) {
            continue;
        }

        // Composants purement axiaux : ne se couplent que si la direction de
        // voisinage correspond a l'axe de rotation commun.
        bool selfAxial = transmitsAlongAxis(self->type());
        bool otherAxial = transmitsAlongAxis(other->type());
        if (selfAxial && otherAxial && !isGearLike(self->type()) && !isGearLike(other->type()) &&
            self->type() != KineticType::Gearbox && other->type() != KineticType::Gearbox) {
            if (self->axis() != d.axis || other->axis() != d.axis) {
                continue;
            }
        }

        Coupling c;
        if (couple(d.axis, self->type(), other->type(), c)) {
            mGraph.connect(selfIt->second, nbIt->second, c, 0);
        }
    }
}

void KineticWorld::refreshConnections() {
    // Reconstruit la topologie a partir de la position actuelle des composants,
    // sans recreer les noeuds (leurs parametres, comme la vitesse du moteur
    // creatif, sont ainsi conserves).
    mGraph.clearConnections();
    for (auto const& [pos, id] : mNodesByPos) {
        (void)id;
        connectTo(pos);
    }
    mGraph.markAllDirty();
}

SolveStats KineticWorld::solve() {
    return mGraph.solve();
}

KineticState const* KineticWorld::stateAt(BlockPos pos) const {
    auto it = mNodesByPos.find(pos);
    if (it == mNodesByPos.end()) {
        return nullptr;
    }
    if (auto const* n = mGraph.node(it->second)) {
        return &n->state();
    }
    return nullptr;
}

NetworkState const* KineticWorld::networkStateAt(BlockPos pos) const {
    auto it = mNodesByPos.find(pos);
    if (it == mNodesByPos.end()) {
        return nullptr;
    }
    if (auto net = mGraph.networkOf(it->second)) {
        return &mGraph.networks()[*net].state();
    }
    return nullptr;
}

} // namespace cl::kinetic
