// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Windows/CreateLeviLaminaMod.h
//
// Classe de mod LeviLamina (Windows), calquee sur le template officiel :
//   https://github.com/LiteLDev/levilamina-mod-template
// load()  : initialisation hors-jeu (config, logs).
// enable(): abonnements aux evenements et enregistrement des commandes.
#pragma once

#if defined(_WIN32)

#include "ll/api/mod/NativeMod.h"

namespace cl::platform::windows {

class CreateLeviLaminaMod {
public:
    static CreateLeviLaminaMod& getInstance();

    CreateLeviLaminaMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    // Initialisation hors-jeu. Appelee au chargement du mod.
    bool load();

    // Abonnements aux evenements (pose/destruction de blocs).
    bool enable();

    // Nettoyage.
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace cl::platform::windows

#endif // _WIN32
