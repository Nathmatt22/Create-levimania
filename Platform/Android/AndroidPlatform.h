// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Android/AndroidPlatform.h
//
// Adaptateur Android du mod, base sur l'API native de LeviLauncher (pl::).
// Implemente IPlatform en s'appuyant sur le monde Bedrock Android.
//
// Compile uniquement sur Android (NDK). Voir :
//   https://docs.levimc.org/LeviLamina/developer_guides/android/
//   https://docs.levimc.org/LeviLamina/developer_guides/android/api/mod/
//
// Le noyau et la couche monde sont identiques a Windows : seule
// l'implementation de IPlatform change.
#pragma once

#include "../../Platform/KineticBridge.h"

#if defined(__ANDROID__)

namespace cl::platform::android {

class AndroidPlatform final : public IPlatform {
public:
    AndroidPlatform() = default;

    std::optional<BlockInfo> getBlock(kinetic::BlockPos pos) const override {
        // TODO(android): lire le bloc via l'API native LeviLauncher.
        (void)pos;
        return std::nullopt;
    }

    void setBlock(kinetic::BlockPos pos, BlockId id, int state) override {
        (void)pos;
        (void)id;
        (void)state;
    }

    void playSound(std::string const& sound) override { (void)sound; }
    void showActionBar(std::string const& text) override { (void)text; }

    void log(std::string const& message) override {
        // TODO(android): __android_log_print(ANDROID_LOG_INFO, "CreateLL", "%s", message.c_str());
        (void)message;
    }
};

} // namespace cl::platform::android

#endif // __ANDROID__
