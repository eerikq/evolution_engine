---@diagnostic disable
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

-- toolchain
set_languages("c++20")

if os.host() == "windows" then
    set_toolchains("msvc")
else
    set_toolchains("clang")
end

------------------ packages ------------------
add_repositories("local-repo xmake")

add_requires("volk 1.4.335+0")
add_requires("vulkan-headers 1.4.335+0")
add_requires("vulkan-memory-allocator 3.3.0")
add_requires("slang 2026.14.1") -- local package

add_requires("tinyobjloader")
-- add_requires("cgltf 1.15") -- implement after obj importing works
-- add_requires("assimp 6.0.5") -- assimp seems cool but i think cgltf does things in a way i like more

add_requires("glm 1.0.3")
add_requires("fmt 12.2.0")
add_requires("stb 2026.03.18")

add_requires("libsdl3 3.4.12")
add_requires("imgui v1.92.7-docking", {
    configs = {
        sdl3 = true,
        sdl3_renderer = true,
        vulkan = true,
        volk = true,
    },
})

------------------ renderer ------------------
target("evolution_renderer")
    set_kind("static")

    add_files("renderer/src/renderer.cpp")
    add_files("renderer/src/platform/platform_sdl3.cpp")
    add_files("renderer/src/gfx/instance.cpp")
    add_files("renderer/src/gfx/device.cpp")
    add_files("renderer/src/gfx/vma.cpp")
    add_files("renderer/src/gfx/swapchain.cpp")
    add_files("renderer/src/gfx/command.cpp")
    add_files("renderer/src/gfx/descriptor.cpp")
    add_files("renderer/src/gfx/resources/buffer.cpp")
    add_files("renderer/src/gfx/resources/texture.cpp")

    add_includedirs("renderer/src")
    add_includedirs("renderer/include", { public = true })

    add_defines("SDL_MAIN_HANDLED", "VK_NO_PROTOTYPES", { public = true })
    add_defines("GLM_FORCE_DEPTH_ZERO_TO_ONE", "GLM_FORCE_RADIANS")
    add_defines("VMA_VULKAN_VERSION=1004000", { public = true })
    add_defines("VMA_STATIC_VULKAN_FUNCTIONS=0", "VMA_DYNAMIC_VULKAN_FUNCTIONS=1", { public = true })
    add_cxflags("-ferror-limit=0", { tools = { "clang", "clangxx" } })

    add_packages("volk", "vulkan-headers", "vulkan-memory-allocator", "slang")
    add_packages("tinyobjloader", "glm", "fmt", "stb")
    add_packages("libsdl3", "imgui")
target_end()

------------------ sandbox ------------------
target("evolution_sandbox")
    set_kind("binary")
    add_deps("evolution_renderer", { whole = true })

    add_files("sandbox/src/**.cpp")
    add_includedirs("sandbox/src")

    add_cxflags("-ferror-limit=0", { tools = { "clang", "clangxx" } })

    after_build( function (target)
        import("core.project.task")

        task.run("copy_assets")
        task.run("compile_shaders")
    end)
target_end()

------------------ tasks ------------------
task("copy_assets")
    on_run( function ()
        local asset_path = path.join(os.projectdir(), "assets")
        local build_path = path.join(os.projectdir(), get_config("builddir"), os.host(), os.arch(), get_config("mode"))

        os.cp(asset_path, build_path)
        print("Moved assets from", asset_path, "to", build_path)
    end)
task_end()

task("compile_shaders")
    on_run( function ()
        local shader_name = "shader.slang"
        local shader_output = "slang.spv"

        local asset_path = path.join(os.projectdir(), "shaders", shader_name)
        local build_path = path.join(os.projectdir(), get_config("builddir"), os.host(), os.arch(), get_config("mode"),
            "shaders")
        local output_file = path.join(build_path, shader_output)

        if not os.exists(build_path) then
            os.mkdir(build_path)
        end

        os.runv("slangc", {
            asset_path,
            "-target", "spirv",
            "-profile", "spirv_1_4",
            "-emit-spirv-directly", "-fvk-use-entrypoint-name",
            "-entry", "vertMain",
            "-entry", "fragMain",
            "-o", output_file,
        })

        print("Compiled", asset_path, "to", output_file)
    end)
task_end()
