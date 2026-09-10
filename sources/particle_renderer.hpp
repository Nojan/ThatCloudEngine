#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "iupdater.hpp"
#include "texture.hpp"
#include "vector.hpp"

#include <glm/glm.hpp>
#include "opengl_includes.hpp"

#include <memory>
#include <vector>

class ParticleData;
class ShaderProgram; 
class ResourceShader;

class ParticleRenderer : public IRenderer, public IUpdater {
public:
    ParticleRenderer();
    ~ParticleRenderer();

    void Update(const float deltaTime) override;
    void Render(const Scene * scene) override;
    void FlushFrame() override;
    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

    void spawnBallParticles(size_t pCount, const glm::vec3& initialPosition, const glm::vec3& initialSpeed, const float speed, const Color::rgbp color, const float lifetime);
    void spawnParticle(const glm::vec3& initialPosition, const glm::vec3& initialSpeed, const float lifetime, const Color::rgbp color);

    void HandleMousePosition(float x, float y, float z);

#if GUI_DEBUG()
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "Particle Renderer"; }

private:
    std::unique_ptr<ParticleData> mParticleData;
    std::unique_ptr<ResourceShader> mShaderResource;
    std::shared_ptr<ShaderProgram> mShaderProgram;
    GLuint mVboPositionId;
    GLuint mVboColorId;
    GLuint mTextureId;
    glm::vec3 mMousePosition;
    Texture2D mParticleMask;
};
