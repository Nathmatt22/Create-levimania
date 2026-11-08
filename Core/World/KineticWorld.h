// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/World/KineticWorld.h
//
// Passerelle entre le monde Minecraft (positions de blocs) et le graphe
// cinematique du noyau. Elle ne depend pas de Minecraft : la couche plateforme
// lui signale simplement "un composant a ete place / casse a telle position".
//
// Responsabilites :
//   * maintenir une correspondance BlockPos <-> NodeId ;
//   * deduire les connexions automatiques entre composants adjacents
//     (arbres alignes bout a bout, engrenages en prise, etc.) ;
//   * exposer l'etat resolu pour le rendu.
#pragma once

#include "../KineticSystem/KineticGraph.h"
#include "../Rotation/Types.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace cl::kinetic {

class KineticWorld {
public:
    KineticWorld() = default;

    // Place un composant a une position. Renvoie l'identifiant du noeud
    // (nouveau, ou existant si la position etait deja occupee).
    NodeId place(BlockPos pos, KineticType type, Axis axis);

    // Retire le composant a une position. Renvoie true si quelque chose a ete
    // retire. Les connexions sont recreees automatiquement.
    bool erase(BlockPos pos);

    // Change l'axe d'un composant existant (ex. rotation de l'arbre).
    bool setAxis(BlockPos pos, Axis axis);

    // Regle la vitesse d'une source (moteur creatif).
    bool setSpeed(BlockPos pos, double rpm);

    bool occupied(BlockPos pos) const { return mNodesByPos.count(pos) != 0; }
    std::optional<NodeId> nodeAt(BlockPos pos) const;

    // Etat resolu pour le rendu.
    KineticState const* stateAt(BlockPos pos) const;
    NetworkState const* networkStateAt(BlockPos pos) const;

    KineticGraph& graph() { return mGraph; }
    KineticGraph const& graph() const { return mGraph; }

    std::size_t componentCount() const { return mNodesByPos.size(); }

    // Recalcule les connexions automatiques pour tout le monde, puis resout.
    SolveStats solve();

private:
    // Reconstruit les connexions en partant de zero (apres un placement/retrait).
    void refreshConnections();

    // Ajoute les connexions d'un composant avec ses voisins.
    void connectTo(BlockPos pos);

    // Determine si deux composants adjacents sur un axe donne doivent etre
    // couples, et par quel mode de couplage.
    static bool couple(Axis sharedAxis, KineticType a, KineticType b, Coupling& out);

    KineticGraph mGraph;
    std::unordered_map<BlockPos, NodeId, BlockPosHash> mNodesByPos;
};

} // namespace cl::kinetic
