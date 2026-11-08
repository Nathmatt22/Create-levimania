// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Windows/WindowsPlatform.h
//
// Adaptateur Windows du mod, base sur l'API LeviLamina (ll::). Il implemente
// IPlatform en s'appuyant sur le monde du serveur/client Bedrock.
//
// Ce fichier n'est compile que sur Windows avec le SDK LeviLamina. Il n'est
// volontairement pas inclus par les tests (qui utilisent une plateforme mock).
//
// Points d'integration LeviLamina utilises (voir docs.levimc.org) :
//   * enregistrement des evenements de monde (BlockPlaced / BlockBroken) ;
//   * lecture/ecriture de blocs via l'API Level/BlockSource ;
//   * affichage d'action bar et lecture de sons cote client.
//
// IMPORTANT : les signatures exactes dependent de la version de LeviLamina.
// Les appels marques TODO doivent etre ajustes a l'installation du SDK ; la
// logique metier, elle, est deja entierement portee dans Core/ et Platform/.
#pragma once

#include "../../Platform/KineticBridge.h"

#if defined(_WIN32)

#include "ll/api/mod/Mod.h"

namespace cl::platform::windows {

// Adaptateur concret. Il conserve une reference au mod LeviLamina pour
// journaliser et pour acceder au monde.
class WindowsPlatform final : public IPlatform {
public:
    explicit WindowsPlatform(ll::mod::Mod& mod) : mMod(mod) {}

    std::optional<BlockInfo> getBlock(kinetic::BlockPos pos) const override {
        // TODO(levilamina): lire le bloc via Level::getBlockSource / Block const&.
        //   auto block = ...->getBlock(pos.x, pos.y, pos.z);
        //   return BlockInfo{ {block.getTypeName()}, 0 };
        (void)pos;
        return std::nullopt;
    }

    void setBlock(kinetic::BlockPos pos, BlockId id, int state) override {
        // TODO(levilamina): ecrire le bloc (utilise pour les pieces mobiles).
        (void)pos;
        (void)id;
        (void)state;
    }

    void playSound(std::string const& sound) override {
        // TODO(levilamina): jouer le son aupres des clients proches.
        (void)sound;
    }

    void showActionBar(std::string const& text) override {
        // TODO(levilamina): envoyer un message client "action bar".
        mMod.getLogger().info("{}", text);
    }

    void log(std::string const& message) override { mMod.getLogger().info("{}", message); }

private:
    ll::mod::Mod& mMod;
};

} // namespace cl::platform::windows

#endif // _WIN32
