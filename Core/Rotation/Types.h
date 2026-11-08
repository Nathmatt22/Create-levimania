// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/ Rotation / Types.h
//
// Fondations geometriques et cinetiques du noyau.
// Ce fichier est volontairement independant de Minecraft, de LeviLamina et
// d'Android : il ne compile que du C++ standard et peut donc etre teste sur
// n'importe quelle plateforme.
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>

namespace cl::kinetic {

// ---------------------------------------------------------------------------
// Constantes de simulation
// ---------------------------------------------------------------------------

// Vitesse de reference utilisee par Create pour le calcul du Stress (RPM).
// Toute la consommation est exprimee relativement a cette vitesse.
inline constexpr double kBaseRotationSpeed = 256.0;

// Tolerance utilisee pour comparer deux vitesses (RPM). En dessous, on
// considere que deux propagations de rotation aboutissent a la meme vitesse.
inline constexpr double kSpeedEpsilon = 1.0e-4;

// Longueur maximale d'une courroie (en blocs), identique a Create.
inline constexpr int kMaxBeltLength = 20;

// ---------------------------------------------------------------------------
// Geometrie minimale
// ---------------------------------------------------------------------------

// Axe de rotation principal d'un composant.
enum class Axis : std::uint8_t { X = 0, Y = 1, Z = 2 };

struct BlockPos {
    int x = 0;
    int y = 0;
    int z = 0;

    friend bool operator==(BlockPos const& a, BlockPos const& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    friend bool operator!=(BlockPos const& a, BlockPos const& b) { return !(a == b); }

    BlockPos offset(int dx, int dy, int dz) const { return {x + dx, y + dy, z + dz}; }

    // Un pas dans une direction cardinale.
    BlockPos relative(Axis axis, int sign) const;
};

// Callback de hachage pour utiliser BlockPos comme cle de table.
struct BlockPosHash {
    std::size_t operator()(BlockPos const& p) const noexcept {
        // Melange simple et stable (x,y,z dans [-2^20, 2^20]).
        std::uint64_t h = static_cast<std::uint64_t>(static_cast<std::uint32_t>(p.x)) * 0x9E3779B185EBCA87ull;
        h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(p.y)) * 0xC2B2AE3D27D4EB4Full;
        h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(p.z)) * 0x165667B19E3779F9ull;
        h ^= h >> 29;
        return static_cast<std::size_t>(h);
    }
};

} // namespace cl::kinetic
