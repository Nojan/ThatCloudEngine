#pragma once

#ifdef __EMSCRIPTEN__
#define WEBGL
#endif

#define IMGUI_ENABLE() 1

#if IMGUI_ENABLE()
#define GUI_DEBUG() 1
#else
#define GUI_DEBUG() 0
#endif
