#pragma once

#include <memory>
#include <vector>

#include "config.hpp"
#include "iresourceowner.hpp"

class IRenderer;
class ShadowRenderer;
class Scene;

class RenderManager : public IResourceOwner {
public:
    RenderManager();
    virtual ~RenderManager();

    void Render(const Scene* scene, int screen_width, int screen_height);
    void FlushFrame();

    void ListResources(std::vector<Resource*>& resources);
    void OnLoad();

#if GUI_DEBUG()
    void debug_GUI() const;
#endif

public:
    std::vector<std::shared_ptr< IRenderer > > mRendererList;
    std::unique_ptr<class FinalRender> mFinalRender;
    std::unique_ptr<ShadowRenderer> mShadowRender;
};