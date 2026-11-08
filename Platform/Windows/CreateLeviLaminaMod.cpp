// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Windows/CreateLeviLaminaMod.cpp
//
// Implementation Windows du mod LeviLamina. C'est ici que les evenements
// Minecraft sont traduits en appels au KineticBridge (noyau portable).
//
// Les blocs cinematiques reutilisent l'addon data-driven (Resources/addon) :
// leur identifiant "create:xxx" est mappe vers un KineticType par Platform.cpp.
//
// Evenerements utilises (SDK LeviLamina) :
//   * PlayerPlacedBlockEvent  : un bloc vient d'etre pose par un joueur.
//   * PlayerDestroyBlockEvent : un bloc vient d'etre casse par un joueur.
// Les classes d'evenements ecoutables sont obligatoirement final.
#include "CreateLeviLaminaMod.h"

#if defined(_WIN32)

#include "../KineticBridge.h"
#include "WindowsPlatform.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/mod/RegisterHelper.h"

#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"

#include <memory>

namespace cl::platform::windows {

namespace {

std::unique_ptr<KineticBridge> gBridge;
std::unique_ptr<WindowsPlatform> gPlatform;

// Convertit une position Minecraft en position du noyau.
kinetic::BlockPos toKernel(::BlockPos const& pos) {
    return kinetic::BlockPos{pos.x, pos.y, pos.z};
}

} // namespace

CreateLeviLaminaMod& CreateLeviLaminaMod::getInstance() {
    static CreateLeviLaminaMod instance;
    return instance;
}

bool CreateLeviLaminaMod::load() {
    auto& logger = getSelf().getLogger();
    logger.info("CreateLeviLamina : chargement du noyau cinematique...");

    gPlatform = std::make_unique<WindowsPlatform>(getSelf());
    gBridge = std::make_unique<KineticBridge>(*gPlatform);
    return true;
}

bool CreateLeviLaminaMod::enable() {
    auto& logger = getSelf().getLogger();
    auto& bus = ll::event::EventBus::getInstance();

    // Pose de bloc : si c'est un composant cinematique, on l'ajoute au graphe.
    bus.emplaceListener<ll::event::PlayerPlacedBlockEvent>(
        [&logger](ll::event::PlayerPlacedBlockEvent& ev) {
            auto blockId = ev.placedBlock().getTypeName();
            if (!gBridge) {
                return;
            }
            gBridge->onBlockPlaced(toKernel(ev.pos()), blockId, 0, kinetic::Axis::X);
            logger.debug("Bloc pose: {}", blockId);
        });

    // Destruction de bloc.
    bus.emplaceListener<ll::event::PlayerDestroyBlockEvent>(
        [](ll::event::PlayerDestroyBlockEvent& ev) {
            if (!gBridge) {
                return;
            }
            gBridge->onBlockRemoved(toKernel(ev.pos()));
        });

    logger.info("CreateLeviLamina active.");
    return true;
}

bool CreateLeviLaminaMod::disable() {
    gBridge.reset();
    gPlatform.reset();
    getSelf().getLogger().info("CreateLeviLamina desactive.");
    return true;
}

// Enregistrement aupres de LeviLamina (macro standard du SDK).
LL_REGISTER_MOD(cl::platform::windows::CreateLeviLaminaMod, cl::platform::windows::CreateLeviLaminaMod::getInstance());

} // namespace cl::platform::windows

#endif // _WIN32
