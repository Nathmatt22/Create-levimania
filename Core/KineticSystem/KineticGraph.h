// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/KineticSystem/KineticGraph.h
//
// Graphe cinematique : stocke les noeuds et leurs connexions, et sait
// resoudre l'etat de chaque reseau. C'est le composant central, independant de
// la plateforme.
#pragma once

#include "KineticNetwork.h"
#include "KineticNode.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace cl::kinetic {

// Mode de couplage entre deux composants adjacents.
enum class Coupling : std::uint8_t {
    Shaft,   // Axe a axe : meme vitesse, meme sens.
    Gear,    // Engrenages qui s'engrenent : rapport de rayon, sens inverse.
    Belt,    // Deux poulies reliees par une courroie : meme vitesse, meme sens.
    Chain,   // Chaine : meme vitesse, sens inverse.
};

struct Connection {
    NodeId a = 0;
    NodeId b = 0;
    Coupling coupling = Coupling::Shaft;
    // Pour une courroie : longueur en blocs (information de rendu/logique).
    int beltLength = 0;
};

// Resultat d'une passe de resolution.
struct SolveStats {
    std::size_t networksSolved = 0;
    std::size_t networksTotal = 0;
    bool anyStalled = false;
};

class KineticGraph {
public:
    NodeId addNode(KineticNode node) {
        NodeId id = mNextId++;
        node.setId(id);
        mNodes.emplace(id, std::move(node));
        mEverythingDirty = true;
        return id;
    }

    bool removeNode(NodeId id);

    KineticNode* node(NodeId id) {
        auto it = mNodes.find(id);
        return it == mNodes.end() ? nullptr : &it->second;
    }
    KineticNode const* node(NodeId id) const {
        auto it = mNodes.find(id);
        return it == mNodes.end() ? nullptr : &it->second;
    }

    void connect(NodeId a, NodeId b, Coupling c, int beltLength = 0);
    void disconnect(NodeId a, NodeId b);
    void clear();

    // Supprime toutes les connexions mais conserve les noeuds. Utilise par la
    // couche monde pour reconstruire la topologie sans recreer les composants.
    void clearConnections();

    std::vector<NodeId> const& neighbours(NodeId id) const;
    std::vector<Connection> const& connections() const { return mConnections; }

    std::size_t nodeCount() const { return mNodes.size(); }
    std::size_t connectionCount() const { return mConnections.size(); }

    // Marque tout le graphe comme a recalculer.
    void markAllDirty() { mEverythingDirty = true; }

    // Marque uniquement le reseau contenant ce noeud comme a recalculer.
    // Utilise quand un parametre (ex. vitesse du moteur) change : on evite de
    // reconstruire tous les reseaux.
    void markNodeDirty(NodeId id);

    // Resout tous les reseaux. Optimise : ne recalcule que les reseaux touches
    // par un changement structurel ou de parametre.
    SolveStats solve();

    // Acces en lecture seule aux reseaux resolus.
    std::vector<KineticNetwork> const& networks() const { return mNetworks; }

    // Reseau auquel appartient un noeud (nullopt si le noeud n'est pas connecte).
    std::optional<std::size_t> networkOf(NodeId id) const;

private:
    void rebuildIfNeeded();
    void solveNetwork(KineticNetwork& net);

    std::uint64_t mNextId = 1;
    std::unordered_map<NodeId, KineticNode> mNodes;
    std::vector<Connection> mConnections;
    struct Edge {
        NodeId other;
        Coupling coupling;
        int beltLength;
    };
    std::unordered_map<NodeId, std::vector<Edge>> mEdgeIndex; // propagation O(V+E)
    std::unordered_map<NodeId, std::vector<NodeId>> mNeighbours;
    std::unordered_map<NodeId, std::size_t> mNetworkIndex;

    bool mEverythingDirty = true;
    std::vector<NodeId> mDirtyNodes; // structure modifiee -> reconstruction
    std::vector<KineticNetwork> mNetworks;
};

} // namespace cl::kinetic
