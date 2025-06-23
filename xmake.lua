-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibob64")


-- set project
set_project("Runtime-EditorIDs64")
set_version("2.0.0")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")


-- setup targets
target("Runtime-EditorIDs64")
    -- bind local dependencies
    add_deps("commonlibob64")

    -- add commonlibob64 plugin
    add_rules("commonlibob64.plugin", {
        name = "Runtime-EditorIDs64",
        author = "shadeMe",
        description = "OBSE64 plugin for runtime editor IDs",
    })

    -- add source files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/PCH.h")

    -- add extra files
    add_extrafiles(".clang-format")
