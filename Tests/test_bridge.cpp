// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Tests/test_bridge.cpp
//
// Teste le pont plateforme <-> noyau avec une IPlatform mock.
#include "../Platform/KineticBridge.h"

#include <cstdio>
#include <string>
#include <unordered_map>

using namespace cl::platform;
using namespace cl::kinetic;

// Plateforme en memoire : un monde factice + un journal des evenements.
class MockPlatform : public IPlatform {
public:
    std::optional<BlockInfo> getBlock(BlockPos pos) const override {
        auto it = mBlocks.find(pos);
        if (it == mBlocks.end()) {
            return std::nullopt;
        }
        return it->second;
    }
    void setBlock(BlockPos pos, BlockId id, int state) override {
        mBlocks[pos] = BlockInfo{id, state};
    }
    void playSound(std::string const& sound) override { lastSound = sound; }
    void showActionBar(std::string const& text) override { lastActionBar = text; }
    void log(std::string const& message) override { lastLog = message; }

    std::unordered_map<BlockPos, BlockInfo, BlockPosHash> mBlocks;
    std::string lastSound;
    std::string lastActionBar;
    std::string lastLog;
};

static int gChecks = 0;
static int gFails = 0;

static void check(bool cond, std::string const& what) {
    ++gChecks;
    if (!cond) {
        ++gFails;
        std::printf("  [ECHEC] %s\n", what.c_str());
    }
}

int main() {
    std::printf("== Tests du pont plateforme CreateLeviLamina ==\n");

    // Correspondance identifiant <-> type.
    {
        check(typeFromBlockId("create:shaft") == KineticType::Shaft, "create:shaft -> Shaft");
        check(typeFromBlockId("create:creative_motor") == KineticType::CreativeMotor, "motor mappe");
        check(!typeFromBlockId("minecraft:stone").has_value(), "bloc inconnu -> nullopt");
        check(blockIdFromType(KineticType::Cogwheel) == "create:cogwheel", "Cogwheel -> create:cogwheel");
    }

    // Placement via le pont met a jour le graphe.
    {
        MockPlatform p;
        KineticBridge bridge(p);
        bridge.onBlockPlaced({0, 0, 0}, "create:creative_motor", 0, Axis::X);
        bridge.onBlockPlaced({1, 0, 0}, "create:shaft", 0, Axis::X);
        check(bridge.world().componentCount() == 2, "2 composants enregistres");
        bridge.setMotorSpeed({0, 0, 0}, 256.0);
        auto* s = bridge.world().stateAt({1, 0, 0});
        check(s && std::abs(s->speed - 256.0) < 1e-6, "arbre entraine a 256 RPM");
        check(!p.lastActionBar.empty(), "info-bulle affichee");
    }

    // Bloc non cinematique ignore.
    {
        MockPlatform p;
        KineticBridge bridge(p);
        bridge.onBlockPlaced({0, 0, 0}, "minecraft:stone", 0, Axis::X);
        check(bridge.world().componentCount() == 0, "bloc non cinematique ignore");
    }

    // Retrait via le pont.
    {
        MockPlatform p;
        KineticBridge bridge(p);
        bridge.onBlockPlaced({0, 0, 0}, "create:creative_motor", 0, Axis::X);
        bridge.onBlockPlaced({1, 0, 0}, "create:shaft", 0, Axis::X);
        bridge.setMotorSpeed({0, 0, 0}, 128.0);
        bridge.onBlockRemoved({0, 0, 0});
        auto* s = bridge.world().stateAt({1, 0, 0});
        check(s && !s->powered, "arbre non alimente apres retrait moteur");
    }

    std::printf("%d verifications, %d echecs\n", gChecks, gFails);
    return gFails == 0 ? 0 : 1;
}
