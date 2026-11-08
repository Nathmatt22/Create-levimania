// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/KineticSystem/KineticTypes.cpp
#include "KineticTypes.h"

namespace cl::kinetic {

ComponentProperties const& propertiesOf(KineticType type) {
    // Les valeurs de Stress sont exprimees en SU comme dans Create :
    //   * le moteur creatif fournit une capacite tres elevee (tres eleve dans
    //     Create : 16384). On garde cette valeur.
    //   * les relais n'ont aucun impact sur le Stress.
    //   * les consommateurs ont un impact croissant avec leur complexite.
    static ComponentProperties const kShaft{ComponentRole::Relay, 1.0, 0.0, 0.0, 0.0, false, "shaft"};
    static ComponentProperties const kCogwheel{ComponentRole::Relay, 1.0, 0.0, 0.0, 0.0, false, "cogwheel"};
    static ComponentProperties const kLargeCogwheel{ComponentRole::Relay, 2.0, 0.0, 0.0, 0.0, false, "large_cogwheel"};
    static ComponentProperties const kGearbox{ComponentRole::Relay, 1.0, 0.0, 0.0, 0.0, false, "gearbox"};
    static ComponentProperties const kBeltPulley{ComponentRole::Relay, 1.0, 0.0, 0.0, 0.0, false, "belt_pulley"};
    static ComponentProperties const kCreativeMotor{ComponentRole::Source, 1.0, 0.0, 16384.0, 256.0, true, "creative_motor"};
    static ComponentProperties const kWaterWheel{ComponentRole::Source, 2.0, 0.0, 512.0, 32.0, false, "water_wheel"};
    static ComponentProperties const kLargeWaterWheel{ComponentRole::Source, 4.0, 0.0, 1024.0, 16.0, false, "large_water_wheel"};
    static ComponentProperties const kHandCrank{ComponentRole::Source, 1.0, 0.0, 128.0, 32.0, false, "hand_crank"};
    static ComponentProperties const kMechanicalPress{ComponentRole::Consumer, 1.0, 8.0, 0.0, 0.0, false, "mechanical_press"};
    static ComponentProperties const kMechanicalMixer{ComponentRole::Consumer, 1.0, 4.0, 0.0, 0.0, false, "mechanical_mixer"};
    static ComponentProperties const kEncasedFan{ComponentRole::Consumer, 1.0, 2.0, 0.0, 0.0, false, "encased_fan"};
    static ComponentProperties const kFlywheel{ComponentRole::Storage, 1.0, 0.0, 0.0, 0.0, false, "flywheel"};
    static ComponentProperties const kUnknown{ComponentRole::Relay, 1.0, 0.0, 0.0, 0.0, false, "unknown"};

    switch (type) {
    case KineticType::Shaft: return kShaft;
    case KineticType::Cogwheel: return kCogwheel;
    case KineticType::LargeCogwheel: return kLargeCogwheel;
    case KineticType::Gearbox: return kGearbox;
    case KineticType::BeltPulley: return kBeltPulley;
    case KineticType::CreativeMotor: return kCreativeMotor;
    case KineticType::WaterWheel: return kWaterWheel;
    case KineticType::LargeWaterWheel: return kLargeWaterWheel;
    case KineticType::HandCrank: return kHandCrank;
    case KineticType::MechanicalPress: return kMechanicalPress;
    case KineticType::MechanicalMixer: return kMechanicalMixer;
    case KineticType::EncasedFan: return kEncasedFan;
    case KineticType::Flywheel: return kFlywheel;
    case KineticType::Unknown:
    default: return kUnknown;
    }
}

} // namespace cl::kinetic
