#include "render_manager.hpp"

#include "irenderer.hpp"
#include "opengl_includes.hpp"
#include "imgui/imgui_header.hpp"

void RenderManager::Render(const Scene* scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& renderer : mRendererList)
    {
        renderer->Render(scene);
    }
}

void RenderManager::FlushFrame()
{
    for (auto& renderer : mRendererList)
    {
        renderer->FlushFrame();
    }
}

void RenderManager::ListResources(std::vector<Resource*>& resources)
{
    for (auto& renderer : mRendererList)
    {
        renderer->ListResources(resources);
    }
}

void RenderManager::OnLoad()
{
    for (auto& renderer : mRendererList)
    {
        renderer->OnLoad();
    }
}

#if GUI_DEBUG()
void RenderManager::debug_GUI() const
{
    if (ImGui::CollapsingHeader("Renderer"))
    {
        for (auto& renderer : mRendererList)
        {
            if (ImGui::CollapsingHeader(renderer->debug_name()))
                renderer->debug_GUI();
        }
    }
}
#endif