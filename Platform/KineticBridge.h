// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/KineticBridge.h
//
// Relie une IPlatform (monde reel) au noyau (KineticWorld). La plateforme
// signale les evenements ("bloc place/casse", "clic sur le moteur") ; le pont
// met a jour le graphe et resout, puis pousse l'etat au rendu.
//
// Ce pont est totalement testable hors Minecraft grace a une IPlatform mock.
#pragma once

#include "../Core/World/KineticWorld.h"
#include "Platform.h"

#include <cstdio>
#include <string>
#include <unordered_map>

namespace cl::platform {

class KineticBridge {
public:
    explicit KineticBridge(IPlatform& platform) : mPlatform(platform) {}

    // Appele quand un bloc est place dans le monde. Si c'est un composant
    // cinematique connu, il est ajoute au graphe et le reseau est resolu.
    void onBlockPlaced(kinetic::BlockPos pos, std::string const& blockId, int state, kinetic::Axis axis) {
        auto type = typeFromBlockId(blockId);
        if (!type) {
            return;
        }
        mWorld.place(pos, *type, axis);
        mLastState[pos] = state;
        resolveAndReport(pos);
    }

    // Appele quand un bloc est casse.
    void onBlockRemoved(kinetic::BlockPos pos) {
        mLastState.erase(pos);
        if (mWorld.erase(pos)) {
            mWorld.solve();
        }
    }

    // Regle la vitesse d'un moteur creatif et resout son reseau.
    bool setMotorSpeed(kinetic::BlockPos pos, double rpm) {
        if (!mWorld.setSpeed(pos, rpm)) {
            return false;
        }
        mWorld.solve();
        return true;
    }

    // Force une passe de resolution.
    kinetic::SolveStats solve() { return mWorld.solve(); }

    kinetic::KineticWorld& world() { return mWorld; }

private:
    void resolveAndReport(kinetic::BlockPos pos) {
        mWorld.solve();
        if (auto const* s = mWorld.stateAt(pos)) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "Reseau %s : %.0f RPM, sens %+d%s",
                          s->powered ? "actif" : "inactif", s->speed, s->direction,
                          s->stalled ? " (bloque)" : "");
            mPlatform.showActionBar(buf);
        }
    }

    IPlatform& mPlatform;
    kinetic::KineticWorld mWorld;
    std::unordered_map<kinetic::BlockPos, int, kinetic::BlockPosHash> mLastState;
};

} // namespace cl::platform
