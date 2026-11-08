#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# CreateLeviLamina - Tools/run_tests.py
#
# Compile et execute tous les tests du noyau portable avec le compilateur
# disponible (g++ ou clang++). Ne necessite ni cmake ni Minecraft.
#
# Usage: python3 Tools/run_tests.py

import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD = os.path.join(ROOT, "build", "tests")

CORE_SOURCES = [
    "Core/Rotation/Types.cpp",
    "Core/KineticSystem/KineticTypes.cpp",
    "Core/KineticSystem/KineticGraph.cpp",
    "Core/World/KineticWorld.cpp",
    "Platform/Platform.cpp",
]

TESTS = [
    "Tests/test_kinetic.cpp",
    "Tests/test_world.cpp",
    "Tests/test_bridge.cpp",
]


def find_compiler():
    for c in ("g++", "clang++"):
        if shutil.which(c):
            return c
    sys.exit("Aucun compilateur g++/clang++ trouve.")


def main():
    cxx = find_compiler()
    os.makedirs(BUILD, exist_ok=True)
    failures = 0

    for test in TESTS:
        name = os.path.splitext(os.path.basename(test))[0]
        out = os.path.join(BUILD, name)
        cmd = [cxx, "-std=c++20", "-O2", "-Wall", "-Wextra", "-I", ROOT,
               os.path.join(ROOT, test)] + [os.path.join(ROOT, s) for s in CORE_SOURCES] + ["-o", out]
        print(f"== Compilation de {name} ==")
        r = subprocess.run(cmd)
        if r.returncode != 0:
            print(f"  Compilation echouee: {name}")
            failures += 1
            continue
        print(f"== Execution de {name} ==")
        r = subprocess.run([out])
        if r.returncode != 0:
            failures += 1

    if failures:
        print(f"\n{failures} suite(s) en echec.")
        return 1
    print("\nToutes les suites passent.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
