-- SPDX-License-Identifier: MIT
--
-- CreateLeviLamina - xmake.lua
--
-- Build du mod natif Windows pour LeviLamina, calque sur le template officiel :
--   https://github.com/LiteLDev/levilamina-mod-template
--
-- Le noyau portable (Core/, Platform/) est compile ici pour Windows. Le meme
-- code est compile pour Android via le NDK (voir Platform/Android et le guide
-- Android de LeviLamina).

add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

-- add_requires("levilamina x.x.x") pour une version precise.
add_requires("levilamina", { configs = { target_type = get_config("target_type") } })

add_requires("levibuildscript")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("CreateLeviLamina")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")

    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")

    add_defines("NOMINMAX")
    add_includedirs("Core", "Platform")

    add_files("Core/**.cpp")
    add_files("Platform/Platform.cpp")
    add_files("Platform/Windows/**.cpp")

    add_packages("levilamina")

    if is_plat("windows") then
        add_defines("UNICODE", "_HAS_CXX23=1")
        set_exceptions("none") -- Evite les conflits avec /EHa.
        add_cxflags("/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            { tools = { "clang_cl" } }
        )
        set_toolchains("clang-cl")
    end

    if is_config("target_type", "server") then
        -- add_includedirs("src-server")
    else
        -- add_includedirs("src-client")
    end
target_end()

-- Cibles de tests du noyau portable (sans LeviLamina), pour les postes de dev.
-- Chaque fichier de test possede son propre main() : une cible par test.
for _, name in ipairs({ "test_kinetic", "test_world", "test_bridge" }) do
    target(name)
        set_kind("binary")
        set_default(false)
        set_languages("c++20")
        add_files("Tests/" .. name .. ".cpp")
        add_files("Core/**.cpp")
        add_files("Platform/Platform.cpp")
        add_includedirs("Core", "Platform")
        if is_plat("windows") then
            add_cxflags("/utf-8")
        else
            add_cxflags("-Wall", "-Wextra")
        end
    target_end()
end
