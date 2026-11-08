// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Tests/test_kinetic.cpp
//
// Tests du noyau cinematique. Framework minimal sans dependance externe :
// chaque test est une lambda enregistree dans une liste et lancee, ce qui
// permet de compiler et executer les tests sur Windows, Linux et Android.
#include "../Core/KineticSystem/KineticGraph.h"
#include "../Core/Rotation/Types.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

using namespace cl::kinetic;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string const& what) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("  [FAIL] %s\n", what.c_str());
    }
}

void checkNear(double a, double b, double eps, std::string const& what) {
    ++g_checks;
    if (std::abs(a - b) > eps) {
        ++g_failures;
        std::printf("  [FAIL] %s (attendu %.4f, obtenu %.4f)\n", what.c_str(), b, a);
    }
}

struct Test {
    char const* name;
    std::function<void()> fn;
};

std::vector<Test>& registry() {
    static std::vector<Test> r;
    return r;
}

struct Registrar {
    Registrar(char const* name, std::function<void()> fn) { registry().push_back({name, std::move(fn)}); }
};

#define TEST(name)                                        \
    static void name();                                   \
    static Registrar reg_##name(#name, name);             \
    static void name()

// ---------------------------------------------------------------------------

TEST(motor_alone_runs) {
    KineticGraph g;
    auto motor = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::Z));
    g.node(motor)->setRequestedSpeed(64.0);
    auto stats = g.solve();
    check(stats.networksTotal == 1, "un noeud isole = un reseau");
    check(g.node(motor)->state().powered, "moteur seul alimente");
    checkNear(g.node(motor)->state().speed, 64.0, 1e-6, "vitesse du moteur");
    check(g.node(motor)->state().direction == 1, "sens positif");
}

TEST(motor_negative_speed) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::Z));
    g.node(m)->setRequestedSpeed(-128.0);
    g.solve();
    checkNear(g.node(m)->state().speed, 128.0, 1e-6, "vitesse absolue");
    check(g.node(m)->state().direction == -1, "sens negatif");
}

TEST(speed_clamped) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::Z));
    g.node(m)->setRequestedSpeed(9999.0);
    g.solve();
    checkNear(g.node(m)->state().speed, 256.0, 1e-6, "vitesse plafonnee a 256");
}

TEST(shaft_chain_same_speed) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    auto s1 = g.addNode(KineticNode({1, 0, 0}, KineticType::Shaft, Axis::X));
    auto s2 = g.addNode(KineticNode({2, 0, 0}, KineticType::Shaft, Axis::X));
    g.node(m)->setRequestedSpeed(32.0);
    g.connect(m, s1, Coupling::Shaft);
    g.connect(s1, s2, Coupling::Shaft);
    g.solve();
    checkNear(g.node(s2)->state().speed, 32.0, 1e-6, "arbre transmet la vitesse");
    check(g.node(s2)->state().direction == 1, "arbre transmet le sens");
    check(g.node(s2)->state().powered, "arbre alimente");
}

TEST(cogwheel_reverses) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::Z));
    auto c1 = g.addNode(KineticNode({1, 0, 0}, KineticType::Cogwheel, Axis::Z));
    auto c2 = g.addNode(KineticNode({2, 0, 0}, KineticType::Cogwheel, Axis::Z));
    g.node(m)->setRequestedSpeed(64.0);
    g.connect(m, c1, Coupling::Shaft);
    g.connect(c1, c2, Coupling::Gear);
    g.solve();
    check(g.node(c1)->state().direction == 1, "engrenage 1 sens +");
    check(g.node(c2)->state().direction == -1, "engrenage 2 sens inverse");
    checkNear(g.node(c2)->state().speed, 64.0, 1e-6, "engrenages meme rayon = meme vitesse");
}

TEST(large_cogwheel_ratio) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::Z));
    auto small = g.addNode(KineticNode({1, 0, 0}, KineticType::Cogwheel, Axis::Z));
    auto large = g.addNode(KineticNode({2, 0, 0}, KineticType::LargeCogwheel, Axis::Z));
    g.node(m)->setRequestedSpeed(64.0);
    g.connect(m, small, Coupling::Shaft);
    g.connect(small, large, Coupling::Gear);
    g.solve();
    // Petit (r=1) -> grand (r=2) : le grand tourne deux fois moins vite.
    checkNear(g.node(large)->state().speed, 32.0, 1e-6, "rapport d'engrenage 2:1");
    check(g.node(large)->state().direction == -1, "grand engrenage inverse");
}

TEST(belt_same_speed_same_direction) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    auto p1 = g.addNode(KineticNode({1, 0, 0}, KineticType::BeltPulley, Axis::X));
    auto p2 = g.addNode(KineticNode({18, 0, 0}, KineticType::BeltPulley, Axis::X));
    g.node(m)->setRequestedSpeed(16.0);
    g.connect(m, p1, Coupling::Shaft);
    g.connect(p1, p2, Coupling::Belt, 17);
    g.solve();
    checkNear(g.node(p2)->state().speed, 16.0, 1e-6, "courroie meme vitesse");
    check(g.node(p2)->state().direction == 1, "courroie meme sens");
}

TEST(disconnect_stops_relay) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    auto s1 = g.addNode(KineticNode({1, 0, 0}, KineticType::Shaft, Axis::X));
    auto s2 = g.addNode(KineticNode({2, 0, 0}, KineticType::Shaft, Axis::X));
    g.node(m)->setRequestedSpeed(32.0);
    g.connect(m, s1, Coupling::Shaft);
    g.connect(s1, s2, Coupling::Shaft);
    g.solve();
    check(g.node(s2)->state().powered, "s2 alimente avant deconnexion");
    g.disconnect(s1, s2);
    g.solve();
    check(!g.node(s2)->state().powered, "s2 plus alimente apres deconnexion");
    checkNear(g.node(s1)->state().speed, 32.0, 1e-6, "s1 toujours alimente");
}

TEST(remove_node_updates_network) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    auto s1 = g.addNode(KineticNode({1, 0, 0}, KineticType::Shaft, Axis::X));
    g.node(m)->setRequestedSpeed(32.0);
    g.connect(m, s1, Coupling::Shaft);
    g.solve();
    check(g.nodeCount() == 2, "2 noeuds");
    check(g.connectionCount() == 1, "1 connexion");
    g.removeNode(s1);
    auto stats = g.solve();
    check(g.nodeCount() == 1, "1 noeud restant");
    check(g.connectionCount() == 0, "0 connexion restante");
    check(stats.networksTotal == 1, "1 reseau restant");
}

TEST(overstress_stalls) {
    KineticGraph g;
    // Roue hydraulique : capacite 512 SU, vitesse nominale 32 RPM.
    auto wheel = g.addNode(KineticNode({0, 0, 0}, KineticType::WaterWheel, Axis::X));
    // Presse : impact 8 SU par RPM normalise. A 32 RPM, charge = 8 * 32/256 = 1 SU.
    // La roue peut donc alimenter beaucoup de presses sans surcharge.
    NodeId prev = wheel;
    for (int i = 0; i < 10; ++i) {
        auto p = g.addNode(KineticNode({i + 1, 0, 0}, KineticType::MechanicalPress, Axis::X));
        g.connect(prev, p, Coupling::Shaft);
        prev = p;
    }
    g.solve();
    check(!g.node(wheel)->state().stalled, "10 presses sur une roue : pas de blocage");
    check(!g.networks().front().state().overstressed, "pas de surcharge");

    // Second reseau : un manivelle (capacite 128) suralimente un seul arbre,
    // mais on force une vitesse elevee : impossible car la manivelle a 32 RPM.
    // On teste plutot la surcharge en reliant deux moteurs creatifs encadrant
    // beaucoup de consommateurs jusqu'a depasser 16384 SU.
    KineticGraph g2;
    auto motor = g2.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    g2.node(motor)->setRequestedSpeed(256.0); // 256 RPM
    prev = motor;
    // A 256 RPM, chaque presse = 8 SU. 16384 / 8 = 2048 presses pour saturer.
    // On en met 2100 pour depasser la capacite du moteur creatif.
    for (int i = 0; i < 2100; ++i) {
        auto p = g2.addNode(KineticNode({i + 1, 0, 0}, KineticType::MechanicalPress, Axis::X));
        g2.connect(prev, p, Coupling::Shaft);
        prev = p;
    }
    g2.solve();
    check(g2.node(motor)->state().stalled, "reseau en surcharge = blocage");
    check(g2.networks().front().state().overstressed, "surcharge detectee");
    check(g2.networks().front().state().stallReason == "overstressed", "raison = overstressed");
}

TEST(conflicting_sources_stall) {
    KineticGraph g;
    auto m1 = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    auto s = g.addNode(KineticNode({1, 0, 0}, KineticType::Shaft, Axis::X));
    auto m2 = g.addNode(KineticNode({2, 0, 0}, KineticType::CreativeMotor, Axis::X));
    g.node(m1)->setRequestedSpeed(64.0);
    g.node(m2)->setRequestedSpeed(-64.0);
    g.connect(m1, s, Coupling::Shaft);
    g.connect(s, m2, Coupling::Shaft);
    g.solve();
    check(g.node(s)->state().stalled, "conflit de sources = blocage");
}

TEST(incremental_solve_skips_clean) {
    KineticGraph g;
    auto m = g.addNode(KineticNode({0, 0, 0}, KineticType::CreativeMotor, Axis::X));
    auto s = g.addNode(KineticNode({1, 0, 0}, KineticType::Shaft, Axis::X));
    g.node(m)->setRequestedSpeed(32.0);
    g.connect(m, s, Coupling::Shaft);
    auto first = g.solve();
    check(first.networksSolved == first.networksTotal, "premier solve resout tout");
    auto second = g.solve();
    check(second.networksSolved == 0, "second solve ne resout rien (rien de modifie)");

    // Changer la vitesse du moteur marque le reseau comme sale.
    g.node(m)->setRequestedSpeed(128.0);
    g.markNodeDirty(m);
    auto third = g.solve();
    check(third.networksSolved == 1, "un reseau recalcule apres changement de vitesse");
    checkNear(g.node(s)->state().speed, 128.0, 1e-6, "nouvelle vitesse propagee");
}

TEST(performance_many_networks) {
    KineticGraph g;
    // 200 reseaux independants de 10 composants chacun = 2000 noeuds.
    for (int n = 0; n < 200; ++n) {
        NodeId prev = 0;
        for (int i = 0; i < 10; ++i) {
            BlockPos p{n * 32, 0, i};
            KineticType t = (i == 0) ? KineticType::CreativeMotor : KineticType::Shaft;
            auto id = g.addNode(KineticNode(p, t, Axis::Z));
            if (i == 0) {
                g.node(id)->setRequestedSpeed(32.0);
            }
            if (prev != 0) {
                g.connect(prev, id, Coupling::Shaft);
            }
            prev = id;
        }
    }
    auto stats = g.solve();
    check(stats.networksTotal == 200, "200 reseaux");
    check(stats.networksSolved == 200, "200 reseaux resolus");
    check(g.nodeCount() == 2000, "2000 noeuds");
}

} // namespace

int main() {
    std::printf("== Tests du noyau cinematique CreateLeviLamina ==\n");
    for (auto& t : registry()) {
        std::printf("- %s\n", t.name);
        t.fn();
    }
    std::printf("\n%d verifications, %d echecs\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
