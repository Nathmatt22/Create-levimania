// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Windows/CreateLeviLaminaMod.cpp
//
// Implementation Windows du mod LeviLamina. C'est ici que les evenements
// Minecraft sont traduits en appels au KineticBridge (noyau portable).
//
// Les blocs cinematiques reutilisent l'addon data-driven (Resources/addon) :
// leur identifiant "create:xxx" est mappe vers un KineticType par Platform.cpp.
#include "CreateLeviLaminaMod.h"

#if defined(_WIN32)

#include "../KineticBridge.h"
#include "WindowsPlatform.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/mod/RegisterHelper.h"

#include <memory>

namespace cl::platform::windows {

namespace {

std::unique_ptr<KineticBridge> gBridge;
std::unique_ptr<WindowsPlatform> gPlatform;

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
    bus.emplaceListener<ll::event::PlayerPlaceBlockEvent>(
        [&logger](ll::event::PlayerPlaceBlockEvent& ev) {
            // TODO(levilamina): extraire la position et l'identifiant du bloc
            // depuis l'evenement selon la version exacte du SDK.
            //   auto pos = ev.pos();               // -> kinetic::BlockPos
            //   auto id  = ev.block().getTypeName(); // "create:shaft"
            //   gBridge->onBlockPlaced(pos, id, 0, axisFrom(ev));
            (void)logger;
        });

    // Destruction de bloc.
    bus.emplaceListener<ll::event::PlayerDestroyBlockEvent>(
        [&logger](ll::event::PlayerDestroyBlockEvent& ev) {
            // TODO(levilamina): extraire la position et appeler onBlockRemoved.
            //   gBridge->onBlockRemoved(ev.pos());
            (void)logger;
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
LL_REGISTER_MOD(CreateLeviLaminaMod, CreateLeviLaminaMod::getInstance());

} // namespace cl::platform::windows

#endif // _WIN32
