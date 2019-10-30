#include "root.hpp"

#include "camera.hpp"
#include "firework.hpp"
#include "particle.hpp"
#include "platform/platform.hpp"
#include "renderer_list.hpp"
#include "particle_renderer.hpp"
#include "billboard_renderer.hpp"
#include "meshRenderer.hpp"
#include "mesh_renderer.hpp"
#include "skinMeshRenderer.hpp"
#include "scene.hpp"
#include "skybox.hpp"
#include "visualdebug.hpp"
#include "gameplay/loopmanager.hpp"
#include "game_system.hpp"
#include "global.hpp"
#include "imgui/imgui_header.hpp"

#include "opengl_includes.hpp"
#include <SDL2/SDL.h>
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
, mRunning(GL_FALSE)
, mFramesCounter(0)
, mFrameDuration(1)
, mFrameLeftover(0)
, mFrameMultiplier(1)
{
}

Root::~Root()
{
}

FireworksManager* Root::GetFireworksManager()
{
    return mFireworkManager.get();
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

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
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
        "SDL Bootstrap", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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
#endif
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    //glEnable(GL_MULTISAMPLE);
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    gl_log_error();

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
    if (!Global::platform()->Ready())
    {
        printf("Platform not ready\n");
        return;
    }

    int windowsWidth, windowsHeight;
    SDL_GetWindowSize(mSDL_ctx->window, &windowsWidth, &windowsHeight);

    mCamera.reset(new Camera());
    mCamera->WindowResize(windowsWidth, windowsHeight);

    mScene.reset(new Scene());

    RendererList* renderList = Global::rendererList();
    {
        std::shared_ptr<Skybox> renderer(Skybox::GenerateCheckered());
        renderList->addRenderer(renderer.get());
        mRendererList.push_back(renderer);
    }
    std::shared_ptr<ParticleRenderer> particleRenderer = std::make_shared<ParticleRenderer>();
    {
        renderList->addRenderer(particleRenderer.get());
        mRendererList.push_back(particleRenderer);
    }
    {
        std::shared_ptr<MeshRenderer> renderer = std::make_shared<MeshRenderer>();
        renderList->addRenderer(renderer.get());
        mRendererList.push_back(renderer);
    }
    mVisualDebugRenderer.reset(new VisualDebugRenderer());
    {
        renderList->addRenderer(mVisualDebugRenderer.get());
        mRendererList.push_back(mVisualDebugRenderer);
    }
    {
        std::shared_ptr<SkinMeshRenderer> renderer = std::make_shared<SkinMeshRenderer>();
        renderList->addRenderer(renderer.get());
        mRendererList.push_back(renderer);
    }
    {
        std::shared_ptr<BillboardRenderer> renderer = std::make_shared<BillboardRenderer>();
        renderList->addRenderer(renderer.get());
        mRendererList.push_back(renderer);
    }
    mFireworkManager.reset(new FireworksManager(renderList->getRenderer<ParticleRenderer>()));
    mGameplayLoopManager.reset(new Gameplay::LoopManager());
    mGameplayLoopManager->Init();

    mUpdaterList.push_back(mCamera);
    mUpdaterList.push_back(particleRenderer);
    mUpdaterList.push_back(mGameplayLoopManager);
    mUpdaterList.push_back(mFireworkManager);

    printf("Engine initialization done\n");
    gl_log_error();
    mRunning = GL_TRUE;
}

void Root::Terminate()
{
    printf("Engine terminate...\n");
    mGameplayLoopManager->Terminate();
    mRendererList.clear();
    mUpdaterList.clear();

    mCamera.reset();
    mFireworkManager.reset();
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
    assert(GL_TRUE == mRunning);
    const std::chrono::milliseconds frameLimiter(16);
    const float frameDuration = frameLimiter.count() / 1000.f;
    float lastFrameDuration = mFrameDuration.count() / 1000.f;
    if (std::chrono::milliseconds(100) < mFrameDuration)
        lastFrameDuration = frameDuration; //abnormal frame duration (breakpoint?)
    lastFrameDuration += mFrameLeftover;
    const auto beginFrame = std::chrono::high_resolution_clock::now();
    //glClearDepth(1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const size_t windowTitleSize = 265;
    char windowTitle[windowTitleSize];
    snprintf(windowTitle, windowTitleSize, "Particle : %lldms", mFrameDuration.count());
    SDL_SetWindowTitle(mSDL_ctx->window, windowTitle);
    SDL_Event e;
    int width, height;
    SDL_GetWindowSize(mSDL_ctx->window, &width, &height);
    while (SDL_PollEvent(&e) != 0) {
        if (SDL_QUIT == e.type) {
            mRunning = false;
            break;
        }
        if (SDL_KEYDOWN == e.type && SDLK_ESCAPE == e.key.keysym.sym) {
            mRunning = false;
            break;
        }
        if (SDL_WINDOWEVENT == e.type && SDL_WINDOWEVENT_RESIZED == e.window.event)
        {
            width = static_cast<int>(e.window.data1);
            height = static_cast<int>(e.window.data2);
            glViewport(0, 0, width, height);
            mCamera->WindowResize(width, height);
        }
        if (SDL_KEYDOWN == e.type && SDLK_SPACE == e.key.keysym.sym)
        {
            // This is game specific. TODO move into mGameplayLoopManager
            SDL_WarpMouseInWindow(mSDL_ctx->window, width / 2, height / 2);
        }
        mCamera->Event(e);
        mGameplayLoopManager->Event(e);
        if (SDL_MOUSEMOTION == e.type)
        {
            const float motionx = static_cast<float>(e.motion.x - (width / 2));
            const float motiony = static_cast<float>(e.motion.y - (height / 2));
            const float halfWidth = static_cast<float>(width / 2);
            const float halfHeight = static_cast<float>(height / 2);
            mGameplayLoopManager->OnMotion(motionx / halfWidth, motiony / halfHeight);
        }
        if (SDL_CONTROLLERAXISMOTION == e.type)
        {
            const int deadzone = 4000;
            const float max_range = numeric_cast<float>(32767 - deadzone);
            const bool value_positive = 0 <= e.caxis.value;
            int value_abs = abs(e.caxis.value);
            value_abs = deadzone < value_abs ? value_abs - deadzone : 0;
            float value = numeric_cast<float>(value_abs) / max_range;
            if (!value_positive) value *= -1.0f;
            if (SDL_CONTROLLER_AXIS_LEFTX == e.caxis.axis)
            {
                mGameplayLoopManager->OnMotion(value, mGameplayLoopManager->Motion().y);
            } 
            else if (SDL_CONTROLLER_AXIS_LEFTY == e.caxis.axis)
            {
                mGameplayLoopManager->OnMotion(mGameplayLoopManager->Motion().x, value);
            }
        }
    }
    IMGUI_ONLY(ImGui_ImplSdl_NewFrame(mSDL_ctx->window));
    for (std::shared_ptr<IUpdater>& updater : mUpdaterList)
    {
        updater->FrameStep();
    }
    Global::gameSytem()->FrameStep();
    if (mFrameMultiplier <= 0)
    {
        lastFrameDuration = 0;
    }
    float playedFrame = 0;
    while (frameDuration <= lastFrameDuration) {
        lastFrameDuration -= frameDuration;
        playedFrame += frameDuration;
        const float frameStep = frameDuration * mFrameMultiplier;
        for (std::shared_ptr<IUpdater>& updater : mUpdaterList)
        {
            updater->Update(frameStep);
        }
        Global::gameSytem()->Update(frameStep);
    }
    for (auto& renderer : mRendererList)
    {
        renderer->Render(mScene.get());
        renderer->FlushFrame();
    }
    mFrameLeftover = lastFrameDuration;
    static bool autoSpawnParticle = false;
    static int autoSpawnParticleFrame = 100;
#ifdef IMGUI_ENABLE
    if (ImGui::Begin("Debug_Info"))
    {
        ImGui::Text("Frame %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Last frame %.3f ms", lastFrameDuration * 1000.f);
        ImGui::SliderFloat("Frame multiplier", &mFrameMultiplier, 0, 10);
        //if (ImGui::CollapsingHeader("OpenGL"))
        //{
        //    static bool wireframe = false;
        //    ImGui::Checkbox("Wireframe", &wireframe);
        //    if (wireframe)
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //    else
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //}
        if (ImGui::CollapsingHeader("Main camera"))
        {
            mCamera->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Scene - light"))
        {
            mScene->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Particle Module"))
        {
            ImGui::Checkbox("auto spawn", &autoSpawnParticle);
            ImGui::SliderInt("spawn each frame", &autoSpawnParticleFrame, 10, 500);
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
    SDL_GL_SwapWindow(mSDL_ctx->window);
    const auto endFrame = std::chrono::high_resolution_clock::now();
    const auto renderingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endFrame - beginFrame);

    ++mFramesCounter;
    //std::this_thread::sleep_for(frameLimiter - renderingDuration);
    const auto endSleep = std::chrono::high_resolution_clock::now();
#ifdef __EMSCRIPTEN__
    mFrameDuration = frameLimiter;
#else
    mFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSleep - beginFrame);
#endif
    if (autoSpawnParticle && mFramesCounter > autoSpawnParticleFrame)
    {
        mFireworkManager->spawnPeony(glm::ballRand(200.f) + glm::vec3(0.f, 400.f, -500.f), 100.f, 3.f);
        mFramesCounter = 0;
    }
}

bool Root::IsRunning()
{
    return (GL_TRUE == mRunning);
}

Camera * Root::GetCamera()
{
    return mCamera.get();
}

VisualDebugRenderer* Root::GetVisualDebugRenderer()
{
    return mVisualDebugRenderer.get();
}
