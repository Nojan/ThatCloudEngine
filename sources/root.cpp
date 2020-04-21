#include "root.hpp"

#include "camera.hpp"
#include "input_controller.hpp"
#include "platform/platform.hpp"
#include "renderer_list.hpp"
#include "resource.hpp"
#include "resourcefile.hpp"
#include "scene.hpp"
#include "visualdebug.hpp"
#include "gameplay/loopmanager.hpp"
#include "game_system.hpp"
#include "global.hpp"
#include "imgui/imgui_header.hpp"

#include "opengl_includes.hpp"
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>

#ifdef WIN32
// Try to use dedicated GPU
extern "C" {
    __declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Constant {
    IMGUI_VAR(DisableFrameStep, false);
    IMGUI_VAR(DisableUpdater, false);
    IMGUI_VAR(DisableRenderer, false);
}

struct SDL_Context {
    SDL_Window *window;
    SDL_GLContext context;
    SDL_GameController *controller = nullptr;
};

Root& Root::Instance()
{
    static Root instance;
    return instance;
}

Root::Root()
: mSDL_ctx(nullptr)
, mState(State::Created)
, mFrameDuration(1)
, mFrameLeftover(0)
, mFrameMultiplier(1)
{
}

Root::~Root()
{
}

void Root::CreateContext()
{
    srand(42);

    mSDL_ctx = new SDL_Context();
    mSDL_ctx->window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#ifdef WEBGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif        
    const int windowsWidth = 800;
    const int windowsHeight = 600;
    mSDL_ctx->window = SDL_CreateWindow(
        "Cloud", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        windowsWidth, windowsHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (nullptr == mSDL_ctx->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_assert(false);
        exit(EXIT_FAILURE);
    }
    SDL_GL_LoadLibrary(nullptr);
    mSDL_ctx->context = SDL_GL_CreateContext(mSDL_ctx->window);
    if (nullptr == mSDL_ctx->context) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
            SDL_GetError());
        SDL_assert(false);
        exit(EXIT_FAILURE);
    }
    printf("OpenGL loaded\n");
#ifdef USE_GLAD
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        printf("Failed to initialize OpenGL context! SDL Error: %s\n",
            SDL_GetError());
        exit(EXIT_FAILURE);
    }
    glad_setup_callback(false, false);
    gl_log_error();
    glad_setup_callback(false, true);
#endif
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    //glEnable(GL_MULTISAMPLE);
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    mInputController = std::make_unique<InputController>();
    mInputController->SetupTouchControl(glm::ivec2(windowsWidth, windowsHeight));

    // Setup ImGui binding
    IMGUI_ONLY(ImGui_ImplSdl_Init(mSDL_ctx->window));

    if(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL could not initialize game controller subsystem! SDL_Error: %s\n", SDL_GetError());
    } else {
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                mSDL_ctx->controller = SDL_GameControllerOpen(i);
                if (mSDL_ctx->controller) {
                    printf("Found a gamepad: %s\n", SDL_GameControllerName(mSDL_ctx->controller));
                    SDL_GameControllerEventState(SDL_ENABLE);
                    break;
                }
            }
        }
    }

    Global::Load();
}

void Root::Init()
{
    std::vector<Resource*> resources;
    if (State::Created == mState)
    {
        mState = State::ResourceLoading;

        mGameplayLoopManager = std::make_shared<Gameplay::LoopManager>();
        mGameplayLoopManager->ListRenderer(mRendererList);
        mVisualDebugRenderer = std::make_shared<VisualDebugRenderer>();
        Global::rendererList()->addRenderer(mVisualDebugRenderer.get());
        mRendererList.push_back(mVisualDebugRenderer);
        {
            RendererList* renderList = Global::rendererList();
            for (auto& renderer : mRendererList)
            {
                renderer->ListResources(resources);
            }
            mGameplayLoopManager->ListResources(resources);
            for (auto& resource : resources)
            {
                resource->Load();
            }
        }
    }
    else if (State::ResourceLoading == mState) 
    {
        RendererList* renderList = Global::rendererList();
        for (auto& renderer : mRendererList)
        {
            renderer->ListResources(resources);
        }
        mGameplayLoopManager->ListResources(resources);
    }

    int resourcesFileCount = 0;
    int resourcesFileLoaded = 0;
    for (size_t ridx = 0; ridx < resources.size(); ++ridx)
    {
        resources[ridx]->GetDependencies(resources);
        Resource* resource = resources[ridx];
        if(ResourceType::File != resource->type())
            continue;
        resourcesFileCount++;
        ResourceFile* rfile = static_cast<ResourceFile*>(resource);
        if (ResourceFile::State::Loaded == rfile->GetState())
        {
            resourcesFileLoaded++;
        }
    }
    
    int windowsWidth, windowsHeight;
    SDL_GetWindowSize(mSDL_ctx->window, &windowsWidth, &windowsHeight);  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, windowsWidth, windowsHeight);
#if IMGUI_ENABLE()
    const int widthMargin = windowsWidth / 6;
    const int heightMargin = windowsHeight / 6;
    ImGui_ImplSdl_NewFrame(mSDL_ctx->window);
    ImGui::SetNextWindowPos(ImVec2(widthMargin, heightMargin), ImGuiCond_Always); 
    ImGui::SetNextWindowSize(ImVec2(windowsWidth - ( 2 * widthMargin), windowsHeight - ( 2 * heightMargin)), ImGuiCond_Always);
    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);     
    if (ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs))
    {
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("Please wait :)");
        if(0 < resourcesFileCount)
        {
            ImGui::ProgressBar(numeric_cast<float>(resourcesFileLoaded) / numeric_cast<float>(resourcesFileCount));
        }
        if (resourcesFileLoaded == resourcesFileCount)
        {
            ImGui::Text("Compiling resources...");
        }
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplSdl_RenderDrawLists(ImGui::GetDrawData());
#endif
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (SDL_QUIT == e.type) {
            exit(EXIT_FAILURE);
            break;
        }
    }
    SDL_GL_SwapWindow(mSDL_ctx->window);

    if (!Global::platform()->Ready())
    {
        return;
    }
    {
        for (auto& renderer : mRendererList)
        {
            renderer->OnLoad();
        }
        mGameplayLoopManager->OnLoad();
    }
    mCamera.reset(new Camera());
    mCamera->WindowResize(windowsWidth, windowsHeight);

    mScene.reset(new Scene());
    mGameplayLoopManager->Init();

    mUpdaterList.push_back(mCamera);
    mUpdaterList.push_back(mGameplayLoopManager);

    printf("Engine initialization done\n");
    gl_log_error();
    mState = State::Running;
}

void Root::Terminate()
{
    printf("Engine terminate...\n");
    mGameplayLoopManager->Terminate();
    mRendererList.clear();
    mUpdaterList.clear();

    mCamera.reset();
    mVisualDebugRenderer.reset();
    mGameplayLoopManager.reset();
    
    Global::Unload();

    if (nullptr != mSDL_ctx->controller)
    {
        SDL_GameControllerClose(mSDL_ctx->controller);
        mSDL_ctx->controller = nullptr;
    }

    IMGUI_ONLY(ImGui_ImplSdl_Shutdown());
    if (mSDL_ctx->window)
        SDL_DestroyWindow(mSDL_ctx->window);
    SDL_Quit();
    mSDL_ctx = nullptr;
}

void Root::Update()
{
    assert(IsRunning());
    const std::chrono::milliseconds frameLimiter(16);
    const float frameDuration = frameLimiter.count() / 1000.f;
    float lastFrameDuration = mFrameDuration.count() / 1000.f;
    if (std::chrono::milliseconds(100) < mFrameDuration)
        lastFrameDuration = frameDuration; //abnormal frame duration (breakpoint?)
    lastFrameDuration += mFrameLeftover;
    const auto beginFrame = std::chrono::high_resolution_clock::now();
    //glClearDepth(1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_Event e;
    int width, height;
    SDL_GetWindowSize(mSDL_ctx->window, &width, &height);
    mInputController->BeginEvents();
    while (SDL_PollEvent(&e) != 0) {
        if (SDL_QUIT == e.type) {
            mState = State::Terminating;
            break;
        }
        if (SDL_KEYDOWN == e.type && SDLK_ESCAPE == e.key.keysym.sym) {
            mState = State::Terminating;
            break;
        }
        if (SDL_WINDOWEVENT == e.type && SDL_WINDOWEVENT_RESIZED == e.window.event)
        {
            width = static_cast<int>(e.window.data1);
            height = static_cast<int>(e.window.data2);
            glViewport(0, 0, width, height);
            mCamera->WindowResize(width, height);
        }
        mInputController->Event(e, glm::ivec2(width, height));
        mCamera->Event(e); 
        mGameplayLoopManager->Event(e);
    }
    mInputController->EndEvents();
    if (mInputController->GetInput().center)
    {
        // This is game specific. TODO move into mGameplayLoopManager
        //SDL_WarpMouseInWindow(mSDL_ctx->window, width / 2, height / 2);
    }
    mCamera->Control(mInputController->GetInput());
    mGameplayLoopManager->Control(mInputController->GetInput());
    IMGUI_ONLY(ImGui_ImplSdl_NewFrame(mSDL_ctx->window));
    const bool disableFrameStep = Constant::DisableFrameStep;
    const bool disableUpdate = Constant::DisableUpdater;
    const bool disableRenderer = Constant::DisableRenderer;
    if (!disableFrameStep)
    {
        for (std::shared_ptr<IUpdater>& updater : mUpdaterList)
        {
            updater->FrameStep();
        }
        Global::gameSytem()->FrameStep();
    }
    if (mFrameMultiplier <= 0)
    {
        lastFrameDuration = 0;
    }
    float playedFrame = 0;
    while (frameDuration <= lastFrameDuration) {
        lastFrameDuration -= frameDuration;
        playedFrame += frameDuration;
        const float frameStep = frameDuration * mFrameMultiplier;
        if (disableUpdate)
            continue;
        for (std::shared_ptr<IUpdater>& updater : mUpdaterList)
        {
            updater->Update(frameStep);
        }
        Global::gameSytem()->Update(frameStep);
        mInputController->Update(frameStep);
    }
    
    for (auto& renderer : mRendererList)
    {
        if (!disableRenderer)
        {
            renderer->Render(mScene.get());
        }
        renderer->FlushFrame();
    }
    IMGUI_ONLY(mInputController->DrawGamepad());
    mFrameLeftover = lastFrameDuration;
#if GUI_DEBUG()
    if (ImGui::Begin("Debug_Info"))
    {
        ImGui::Text("Frame %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Last frame %.3f ms", lastFrameDuration * 1000.f);
        ImGui::SliderFloat("Frame multiplier", &mFrameMultiplier, 0, 10);
        if (false)
        {
            ImGui::Checkbox("DisableFrameStep", &Constant::DisableFrameStep);
            ImGui::Checkbox("DisableUpdater", &Constant::DisableUpdater);
            ImGui::Checkbox("DisableRenderer", &Constant::DisableRenderer);
        }
        //if (ImGui::CollapsingHeader("OpenGL"))
        //{
        //    static bool wireframe = false;
        //    ImGui::Checkbox("Wireframe", &wireframe);
        //    if (wireframe)
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //    else
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //}
        if (ImGui::CollapsingHeader("Control"))
        {
            mInputController->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Main camera"))
        {
            mCamera->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Scene - light"))
        {
            mScene->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Renderer"))
        {
            for (auto& renderer : mRendererList)
            {
                if (ImGui::CollapsingHeader(renderer->debug_name()))
                    renderer->debug_GUI();
            }
        }
        Global::gameSytem()->debug_GUI();
        if (ImGui::CollapsingHeader("Gameplay"))
        {
            mGameplayLoopManager->debug_GUI();
        }
    }
    ImGui::End();
#endif
    IMGUI_ONLY(ImGui::Render());
    IMGUI_ONLY(ImGui_ImplSdl_RenderDrawLists(ImGui::GetDrawData()));
    SDL_GL_SwapWindow(mSDL_ctx->window);
    if(0 == SDL_GL_GetSwapInterval())
    {
        const auto endFrame = std::chrono::high_resolution_clock::now();
        const auto renderingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endFrame - beginFrame);
        std::this_thread::sleep_for(frameLimiter - renderingDuration);
    }
    const auto endSleep = std::chrono::high_resolution_clock::now();
#ifdef __EMSCRIPTEN__
    mFrameDuration = frameLimiter;
#else
    mFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSleep - beginFrame);
#endif
}

bool Root::IsRunning()
{
    return (State::Running == mState);
}

Camera * Root::GetCamera()
{
    return mCamera.get();
}

VisualDebugRenderer* Root::GetVisualDebugRenderer()
{
    return mVisualDebugRenderer.get();
}
