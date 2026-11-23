#include <evo/core/log.h>
#include <evo/platform.h>
#include <evo/renderer.h>
#include <exception>

// custom ifndefs to avoid including <stdlib.h>
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

int main() {
    platform::Window window;
    renderer::Camera camera;
    renderer::Renderer* renderer = nullptr;

    try {
        platform::WindowDesc window_descriptor = {
            .app_name = "Evolution Engine",
            .app_identifier = "eerikq.evolution_engine",
            .app_version = "0.0.1",
            .default_width = 1200,
            .default_height = 900,
        };

        renderer::RendererDesc renderer_desc = {
            .required_validation_layers = {"VK_LAYER_KHRONOS_validation"},
            .required_device_extensions = {"VK_KHR_swapchain"},
        };

        if (!platform::Init(&window_descriptor)) return EXIT_FAILURE;
        if (!platform::CreateWindow(&window, &window_descriptor)) return EXIT_FAILURE;

        if (!renderer::Create(&window, &renderer_desc, &renderer)) return EXIT_FAILURE;
        renderer::CreateResources(renderer);
        renderer::imgui::CreateContext(&window, renderer);

        while (!window.should_close) {
            platform::PollEvents(&window, &camera);

            if (window.was_resized) {
                platform::ResizeWindow(&window);
                renderer::Resize(&window, renderer);
            }

            renderer::imgui::Process(renderer, &camera);
            renderer::DrawFrame(&window, renderer, &camera);
        }

        renderer::imgui::DestroyContext(renderer);
        renderer::DestroyResources(renderer);
        renderer::Destroy(&window, &renderer);

        platform::DestroyWindow(&window);
        platform::Quit();
    } catch (const std::exception& e) {
        core::Log("Exception: {%s}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
