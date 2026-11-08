-- SPDX-License-Identifier: MIT
--
-- CreateLeviLamina - xmake.lua
--
-- Build du mod natif Windows pour LeviLamina. Requiert le SDK LeviLamina.
-- Voir https://docs.levimc.org/LeviLamina/developer_guides/how_to_guides/build_guide/
--
-- Le noyau portable (Core/, Platform/) est compile ici pour Windows. Le meme
-- code est compile pour Android via le NDK (voir Platform/Android et le guide
-- Android de LeviLamina).

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" })

set_languages("c++20")
set_encodings("utf-8")

-- Optionnel : activer le deploiement local vers l'instance LeviLauncher.
option("deploy")
    set_default(false)
    set_showmenu(true)
option_end()

add_requires("levilamina 1.0.0", { configs = { target_type = "client" } })

target("CreateLeviLamina")
    set_kind("shared")
    set_basename("CreateLeviLamina")

    -- Sources du noyau partage (identiques a la cible Android).
    add_files("Core/**.cpp")
    add_files("Platform/Platform.cpp")
    add_files("Platform/Windows/**.cpp")

    add_includedirs("Core", "Platform")

    add_packages("levilamina")

    add_defines("NOMINMAX")

    if is_plat("windows") then
        add_cxflags("/utf-8", "/W4")
        add_ldflags("/DELAYLOAD:bedrock_server.dll")
    end

    -- Deploiement local si demande : vers l'instance courante.
    if has_config("deploy") then
        on_install(function(target)
            import("lib.detect.find_tool")
            local mods_dir = os.getenv("LEVILAMINA_MODS_DIR")
            if mods_dir then
                os.cp(target:targetfile(), path.join(mods_dir, target:basename() .. ".dll"))
            end
        end)
    end
target_end()

-- Cible de tests du noyau portable (sans LeviLamina), pour les postes de dev.
-- Chaque fichier de test possede son propre main() : une cible par test.
for _, name in ipairs({ "test_kinetic", "test_world", "test_bridge" }) do
    target(name)
        set_kind("binary")
        set_default(false)
        add_files("Tests/" .. name .. ".cpp")
        add_files("Core/**.cpp")
        add_files("Platform/Platform.cpp")
        add_includedirs("Core", "Platform")
        if is_plat("windows") then
            add_cxflags("/utf-8", "/W4")
        else
            add_cxflags("-Wall", "-Wextra")
        end
    target_end()
end
