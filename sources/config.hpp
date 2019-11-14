#pragma once

#ifdef __EMSCRIPTEN__
#define WEBGL
#endif

#define IMGUI_ENABLE() 1

#if IMGUI_ENABLE()
#ifdef __EMSCRIPTEN__
#define  GUI_DEBUG() 0
#else
#define GUI_DEBUG() 1
#endif
#else
#define GUI_DEBUG() 0
#endif
