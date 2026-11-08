// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Windows/CreateLeviLaminaMod.h
//
// Classe de mod LeviLamina (Windows). Suit la structure du template officiel :
// load() pour l'initialisation hors-jeu, enable() pour les abonnements aux
// evenements et l'enregistrement des commandes.
//
// Voir https://docs.levimc.org/LeviLamina/developer_guides/tutorials/create_your_first_mod/
#pragma once

#if defined(_WIN32)

#include "ll/api/mod/Mod.h"

namespace cl::platform::windows {

class CreateLeviLaminaMod {
public:
    // Initialisation hors-jeu (config, logs). Appelee au chargement du mod.
    bool load();

    // Abonnements aux evenements (pose/destruction de blocs, clic sur moteur).
    bool enable();

    // Nettoyage.
    bool disable();

    // Acces a l'instance unique (utilisee par LL_REGISTER_MOD).
    static CreateLeviLaminaMod& getInstance();

    ll::mod::Mod& getSelf() { return *mSelf; }

private:
    ll::mod::Mod* mSelf = nullptr;
};

} // namespace cl::platform::windows

#endif // _WIN32
