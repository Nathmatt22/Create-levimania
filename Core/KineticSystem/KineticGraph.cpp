// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/KineticSystem/KineticGraph.cpp
//
// Implementation du graphe cinematique et du solveur de reseaux.
//
// Algorithme (inspire de Create, adapte a un usage Bedrock) :
//   1. On decoupe le graphe en composantes connexes (une composante = un
//      reseau mecanique).
//   2. Pour chaque reseau on determine une vitesse et un sens de reference en
//      propageant depuis les sources, en appliquant les rapports et inversions
//      propres a chaque type de couplage (engrenages, courroies, chaines).
//   3. On verifie la coherence : tout le reseau doit aboutir a la meme vitesse.
//      Une incoherence (ex. deux sources a des vitesses differentes) bloque le
//      reseau plutot que de clignoter.
//   4. On agrege capacite et charge ; si la charge depasse la capacite, le
//      reseau est en surcharge et bloque (comme le Stress de Create).
#include "KineticGraph.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace cl::kinetic {

namespace {

// Ratio de rayon entre a et b pour un couplage donne. La vitesse de b en
// fonction de a vaut speedA * ratio, avec un signe eventuellement inverse.
struct CouplingResult {
    double speedRatio; // speedB = speedA * speedRatio
    bool invertSign;
};

CouplingResult couplingBetween(KineticNode const& a, KineticNode const& b, Coupling c) {
    switch (c) {
    case Coupling::Shaft:
        return {1.0, false};
    case Coupling::Gear: {
        // Engrenage : le rapport est l'inverse du rapport des rayons, et le
        // sens s'inverse. Un petit engrenage (1) face a un grand (2) fait
        // tourner le grand deux fois moins vite.
        double ra = a.properties().radius;
        double rb = b.properties().radius;
        double ratio = (rb > 0.0) ? (ra / rb) : 1.0;
        return {ratio, true};
    }
    case Coupling::Belt:
        // Courroie : meme vitesse, meme sens. On ne gere pas encore les
        // courroies croisees (inversion), qui viendront en phase 7.
        return {1.0, false};
    case Coupling::Chain:
        // Chaine : meme vitesse, sens inverse.
        return {1.0, true};
    }
    return {1.0, false};
}

} // namespace

void KineticGraph::clear() {
    mNodes.clear();
    mConnections.clear();
    mEdgeIndex.clear();
    mNeighbours.clear();
    mNetworkIndex.clear();
    mNetworks.clear();
    mDirtyNodes.clear();
    mEverythingDirty = true;
    mNextId = 1;
}

void KineticGraph::clearConnections() {
    mConnections.clear();
    mEdgeIndex.clear();
    mNeighbours.clear();
    mNetworkIndex.clear();
    mNetworks.clear();
    mDirtyNodes.clear();
    mEverythingDirty = true;
}

bool KineticGraph::removeNode(NodeId id) {
    if (mNodes.erase(id) == 0) {
        return false;
    }
    // Supprime toutes les connexions touchant ce noeud.
    mConnections.erase(
        std::remove_if(mConnections.begin(), mConnections.end(),
                       [id](Connection const& c) { return c.a == id || c.b == id; }),
        mConnections.end());
    mEdgeIndex.clear();
    mNeighbours.clear();
    // La topologie a change : reconstruction complete requise.
    mEverythingDirty = true;
    return true;
}

void KineticGraph::connect(NodeId a, NodeId b, Coupling c, int beltLength) {
    if (a == b || !mNodes.count(a) || !mNodes.count(b)) {
        return;
    }
    // Evite les doublons (meme paire, meme couplage).
    for (auto const& existing : mConnections) {
        if ((existing.a == a && existing.b == b) || (existing.a == b && existing.b == a)) {
            return;
        }
    }
    mConnections.push_back({a, b, c, beltLength});
    mEdgeIndex.clear();
    mNeighbours.clear();
    mEverythingDirty = true;
}

void KineticGraph::disconnect(NodeId a, NodeId b) {
    auto before = mConnections.size();
    mConnections.erase(
        std::remove_if(mConnections.begin(), mConnections.end(),
                       [a, b](Connection const& c) {
                           return (c.a == a && c.b == b) || (c.a == b && c.b == a);
                       }),
        mConnections.end());
    if (before != mConnections.size()) {
        mEdgeIndex.clear();
        mNeighbours.clear();
        mEverythingDirty = true;
    }
}

std::vector<NodeId> const& KineticGraph::neighbours(NodeId id) const {
    static std::vector<NodeId> const empty;
    auto it = mNeighbours.find(id);
    return it == mNeighbours.end() ? empty : it->second;
}

void KineticGraph::markNodeDirty(NodeId id) {
    auto it = mNetworkIndex.find(id);
    if (it != mNetworkIndex.end() && it->second < mNetworks.size()) {
        mNetworks[it->second].markDirty();
    }
    if (auto* n = node(id)) {
        n->touch();
    }
}

void KineticGraph::rebuildIfNeeded() {
    if (!mEverythingDirty) {
        return;
    }

    // Reconstruit la liste d'adjacence (noeuds voisins + index des aretes).
    mNeighbours.clear();
    mEdgeIndex.clear();
    for (auto const& c : mConnections) {
        mNeighbours[c.a].push_back(c.b);
        mNeighbours[c.b].push_back(c.a);
        mEdgeIndex[c.a].push_back({c.b, c.coupling, c.beltLength});
        mEdgeIndex[c.b].push_back({c.a, c.coupling, c.beltLength});
    }

    // Decoupe en composantes connexes via un parcours en largeur.
    mNetworks.clear();
    mNetworkIndex.clear();
    std::unordered_set<NodeId> seen;
    std::uint64_t netId = 1;

    for (auto const& [id, node] : mNodes) {
        if (seen.count(id)) {
            continue;
        }
        KineticNetwork net(netId++);
        std::queue<NodeId> q;
        q.push(id);
        seen.insert(id);
        while (!q.empty()) {
            NodeId cur = q.front();
            q.pop();
            net.addNode(cur);
            mNetworkIndex[cur] = mNetworks.size();
            for (NodeId nb : neighbours(cur)) {
                if (!seen.count(nb)) {
                    seen.insert(nb);
                    q.push(nb);
                }
            }
        }
        // Chaque noeud isole constitue aussi un reseau (utile pour un moteur
        // creatif non connecte).
        mNetworks.push_back(std::move(net));
    }

    mEverythingDirty = false;
    mDirtyNodes.clear();
}

void KineticGraph::solveNetwork(KineticNetwork& net) {
    NetworkState state;
    state.valid = true;

    auto const& ids = net.nodes();

    // --- 1. Propager vitesse et sens depuis les sources ---------------------
    // On utilise une file et des valeurs de reference. Chaque noeud recoit une
    // vitesse signee. On parcourt le reseau depuis chaque source.
    std::unordered_map<NodeId, double> signedSpeed; // vitesse signee (RPM)
    std::unordered_map<NodeId, bool> assigned;

    // Recherche de la premiere source du reseau.
    std::vector<NodeId> sources;
    double totalCapacity = 0.0;
    for (NodeId id : ids) {
        auto const* n = node(id);
        if (!n) {
            continue;
        }
        if (n->role() == ComponentRole::Source) {
            sources.push_back(id);
            totalCapacity += n->properties().stressCapacity;
        }
    }

    // Propagation a partir de la premiere source (ou d'une racine arbitraire
    // pour un reseau sans source, qui restera a l'arret).
    NodeId root = sources.empty() ? (ids.empty() ? 0 : ids.front()) : sources.front();

    std::queue<NodeId> q;
    if (root != 0 && node(root)) {
        double rootSpeed = 0.0;
        if (node(root)->role() == ComponentRole::Source) {
            rootSpeed = node(root)->requestedSpeed();
        }
        signedSpeed[root] = rootSpeed;
        assigned[root] = true;
        q.push(root);
    }

    // Index des connexions par extremite pour retrouver le couplage.
    while (!q.empty()) {
        NodeId cur = q.front();
        q.pop();
        auto const* curNode = node(cur);
        if (!curNode) {
            continue;
        }
        auto eit = mEdgeIndex.find(cur);
        if (eit == mEdgeIndex.end()) {
            continue;
        }
        for (auto const& edge : eit->second) {
            NodeId other = edge.other;
            auto const* otherNode = node(other);
            if (!otherNode) {
                continue;
            }
            auto res = couplingBetween(*curNode, *otherNode, edge.coupling);
            // speedB = speedA * ratio ; si le signe doit s'inverser, on inverse
            // egalement le signe de la vitesse de reference.
            double factor = res.speedRatio * (res.invertSign ? -1.0 : 1.0);
            double proposed = signedSpeed[cur] * factor;

            if (auto it = assigned.find(other); it != assigned.end()) {
                // Deja assigne : verifie la coherence.
                if (std::abs(proposed - signedSpeed[other]) > kSpeedEpsilon) {
                    // Deux chemins donnent des vitesses differentes : reseau
                    // incoherent, on ne clignote pas, on bloque.
                    state.valid = false;
                    state.stalled = true;
                    state.stallReason = "inconsistent_speed";
                }
            } else {
                assigned[other] = true;
                signedSpeed[other] = proposed;
                q.push(other);
            }
        }
    }

    // --- 2. Verifier que toutes les sources imposent la meme vitesse -------
    for (NodeId id : sources) {
        auto const* n = node(id);
        if (!n) {
            continue;
        }
        double s = n->requestedSpeed();
        if (auto it = signedSpeed.find(id); it != signedSpeed.end()) {
            if (std::abs(it->second - s) > kSpeedEpsilon) {
                state.valid = false;
                state.stalled = true;
                state.stallReason = "conflicting_sources";
            }
        }
    }

    // --- 3. Determiner la vitesse et le sens du reseau ---------------------
    // On choisit arbitrairement un noeud affecte non nul pour definir la
    // reference du reseau.
    double refSpeed = 0.0;
    for (NodeId id : ids) {
        if (auto it = signedSpeed.find(id); it != signedSpeed.end() && std::abs(it->second) > kSpeedEpsilon) {
            refSpeed = it->second;
            break;
        }
    }
    state.speed = std::abs(refSpeed);
    state.direction = (refSpeed > kSpeedEpsilon) ? 1 : (refSpeed < -kSpeedEpsilon ? -1 : 0);

    // --- 4. Agreger la charge ----------------------------------------------
    double load = 0.0;
    for (NodeId id : ids) {
        auto const* n = node(id);
        if (!n || n->role() != ComponentRole::Consumer) {
            continue;
        }
        // Impact en SU par RPM, normalise par la vitesse de base comme Create.
        load += n->properties().stressImpact * (state.speed / kBaseRotationSpeed);
    }
    state.capacity = totalCapacity;
    state.load = load;

    if (!sources.empty() && load > state.capacity + 1e-6) {
        state.overstressed = true;
        state.stalled = true;
        state.valid = false;
        state.stallReason = "overstressed";
    }

    if (sources.empty()) {
        // Aucune source : le reseau est simplement a l'arret (pas bloque).
        state.valid = true;
        state.stalled = false;
    }

    if (!state.stalled && state.direction == 0) {
        // Source presente mais vitesse nulle : arret normal.
        state.valid = sources.empty() ? true : true;
    }

    // --- 5. Ecrire l'etat dans chaque noeud --------------------------------
    for (NodeId id : ids) {
        auto* n = node(id);
        if (!n) {
            continue;
        }
        KineticState ks;
        ks.powered = !sources.empty() && !state.stalled;
        ks.stalled = state.stalled;
        ks.speed = state.speed;
        double signedRef = 0.0;
        if (auto it = signedSpeed.find(id); it != signedSpeed.end()) {
            signedRef = it->second;
        }
        ks.direction = (signedRef > kSpeedEpsilon) ? 1 : (signedRef < -kSpeedEpsilon ? -1 : 0);
        if (ks.direction == 0 && state.direction != 0 && state.speed > kSpeedEpsilon) {
            // Noeud non atteint par la propagation : on garde le sens du reseau
            // pour rester coherent visuellement.
            ks.direction = state.direction;
        }
        n->setState(ks);
    }

    net.setState(state);
}

SolveStats KineticGraph::solve() {
    rebuildIfNeeded();

    SolveStats stats;
    stats.networksTotal = mNetworks.size();

    for (auto& net : mNetworks) {
        if (!net.dirty()) {
            continue;
        }
        solveNetwork(net);
        net.clearDirty();
        ++stats.networksSolved;
        if (net.state().stalled) {
            stats.anyStalled = true;
        }
    }
    return stats;
}

std::optional<std::size_t> KineticGraph::networkOf(NodeId id) const {
    auto it = mNetworkIndex.find(id);
    if (it == mNetworkIndex.end()) {
        return std::nullopt;
    }
    return it->second;
}

} // namespace cl::kinetic
