#pragma once

#ifdef __EMSCRIPTEN__
#define WEBGL
#endif

#define IMGUI_ENABLE

#ifdef IMGUI_ENABLE
#define GUI_DEBUG() 1
#else
#define GUI_DEBUG() 0
#endif // IMGUI_ENABLE
