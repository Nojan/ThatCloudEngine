#pragma once

#include "config.hpp"
#include "iresourceowner.hpp"

class Scene;

class IRenderer : public IResourceOwner {
public:
	
    virtual void Render(const Scene* scene) = 0;
    virtual void FlushFrame() = 0;

#if GUI_DEBUG()
    virtual void debug_GUI() const;
#endif
    virtual const char* debug_name() const = 0;

};

#if GUI_DEBUG()
inline void IRenderer::debug_GUI() const {}
#endif
