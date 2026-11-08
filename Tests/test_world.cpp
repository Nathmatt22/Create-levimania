// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Tests/test_world.cpp
//
// Tests de la couche monde : placement, couplage automatique, retrait.
#include "../Core/World/KineticWorld.h"

#include <cstdio>
#include <string>

using namespace cl::kinetic;

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
    std::printf("== Tests de la couche monde CreateLeviLamina ==\n");

    // 1. Moteur seul sur son axe.
    {
        KineticWorld w;
        w.place({0, 0, 0}, KineticType::CreativeMotor, Axis::X);
        w.setSpeed({0, 0, 0}, 128.0);
        w.solve();
        auto* s = w.stateAt({0, 0, 0});
        check(s && std::abs(s->speed - 128.0) < 1e-6, "moteur seul tourne a 128 RPM");
        check(s && s->powered, "moteur alimente");
    }

    // 2. Arbre aligne bout a bout -> meme vitesse.
    {
        KineticWorld w;
        w.place({0, 0, 0}, KineticType::CreativeMotor, Axis::X);
        w.place({1, 0, 0}, KineticType::Shaft, Axis::X);
        w.place({2, 0, 0}, KineticType::Shaft, Axis::X);
        w.setSpeed({0, 0, 0}, 256.0);
        w.solve();
        auto* s2 = w.stateAt({2, 0, 0});
        check(s2 && std::abs(s2->speed - 256.0) < 1e-6, "arbre lointain a 256 RPM");
    }

    // 3. Arbre perpendiculaire non couple (axes differents).
    {
        KineticWorld w;
        w.place({0, 0, 0}, KineticType::CreativeMotor, Axis::X);
        w.place({0, 1, 0}, KineticType::Shaft, Axis::X); // decale en Y -> pas dans l'axe
        w.setSpeed({0, 0, 0}, 256.0);
        w.solve();
        auto* s = w.stateAt({0, 1, 0});
        check(s && std::abs(s->speed) < 1e-6, "arbre hors axe non entraine");
    }

    // 4. Engrenage inverse le sens de l'arbre.
    {
        KineticWorld w;
        w.place({0, 0, 0}, KineticType::CreativeMotor, Axis::X);
        w.place({1, 0, 0}, KineticType::Shaft, Axis::X);
        w.place({2, 0, 0}, KineticType::Cogwheel, Axis::X);
        w.setSpeed({0, 0, 0}, 256.0);
        w.solve();
        auto* motor = w.stateAt({0, 0, 0});
        auto* gear = w.stateAt({2, 0, 0});
        check(motor && gear, "noeuds presents");
        if (motor && gear) {
            check(gear->direction == -motor->direction, "engrenage inverse le sens");
        }
    }

    // 5. Retrait du moteur : plus rien ne tourne.
    {
        KineticWorld w;
        w.place({0, 0, 0}, KineticType::CreativeMotor, Axis::X);
        w.place({1, 0, 0}, KineticType::Shaft, Axis::X);
        w.setSpeed({0, 0, 0}, 200.0);
        w.solve();
        bool powered = w.stateAt({1, 0, 0})->powered;
        w.erase({0, 0, 0});
        w.solve();
        auto* s = w.stateAt({1, 0, 0});
        check(powered, "arbre alimente avant retrait");
        check(s && !s->powered && std::abs(s->speed) < 1e-6, "arbre arrete apres retrait du moteur");
    }

    // 6. Nouveau composant connecte apres coup : recalcul automatique.
    {
        KineticWorld w;
        w.place({0, 0, 0}, KineticType::CreativeMotor, Axis::X);
        w.setSpeed({0, 0, 0}, 256.0);
        w.solve();
        w.place({1, 0, 0}, KineticType::Shaft, Axis::X);
        w.solve();
        auto* s = w.stateAt({1, 0, 0});
        check(s && std::abs(s->speed - 256.0) < 1e-6, "arbre ajoute apres coup entraine");
    }

    std::printf("%d verifications, %d echecs\n", gChecks, gFails);
    return gFails == 0 ? 0 : 1;
}
