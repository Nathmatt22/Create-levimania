// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Platform/Platform.cpp
//
// Correspondance entre identifiants de blocs Bedrock ("create:shaft") et le
// type cinetique du noyau. Cette table est le seul endroit a modifier pour
// ajouter un nouveau composant.
#include "Platform.h"

#include <unordered_map>

namespace cl::platform {

using kinetic::KineticType;

namespace {

struct Entry {
    char const* blockId;
    KineticType type;
};

// Aligne sur les identifiants "create:" du mod d'origine.
constexpr Entry kTable[] = {
    {"create:shaft", KineticType::Shaft},
    {"create:cogwheel", KineticType::Cogwheel},
    {"create:large_cogwheel", KineticType::LargeCogwheel},
    {"create:gearbox", KineticType::Gearbox},
    {"create:belt", KineticType::BeltPulley},
    {"create:creative_motor", KineticType::CreativeMotor},
    {"create:water_wheel", KineticType::WaterWheel},
    {"create:large_water_wheel", KineticType::LargeWaterWheel},
    {"create:hand_crank", KineticType::HandCrank},
    {"create:mechanical_press", KineticType::MechanicalPress},
    {"create:mechanical_mixer", KineticType::MechanicalMixer},
    {"create:encased_fan", KineticType::EncasedFan},
    {"create:flywheel", KineticType::Flywheel},
};

} // namespace

std::optional<KineticType> typeFromBlockId(std::string const& id) {
    for (auto const& e : kTable) {
        if (id == e.blockId) {
            return e.type;
        }
    }
    return std::nullopt;
}

std::string blockIdFromType(KineticType type) {
    for (auto const& e : kTable) {
        if (e.type == type) {
            return e.blockId;
        }
    }
    return "create:unknown";
}

} // namespace cl::platform
