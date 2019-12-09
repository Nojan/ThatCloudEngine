#pragma once

#include "ressourcecache.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"

class ShaderCache : public RessourceCache<ShaderProgram>
{
public:
    void get_dependencies(const std::string& name, std::vector<std::string>& dependencies) const override
    {
        dependencies.reserve(dependencies.size() + 2);
        const size_t string_length_max = 2048;
        char shader_path[string_length_max];
        snprintf(shader_path, string_length_max, "../shaders/%s.vert", name.c_str());
        dependencies.push_back(std::string(shader_path));
        snprintf(shader_path, string_length_max, "../shaders/%s.frag", name.c_str());
        dependencies.push_back(std::string(shader_path));
    }

protected:
    std::shared_ptr<ShaderProgram> load(const std::string& name) const override
    {
        std::vector<std::string> dependencies;
        get_dependencies(name, dependencies);
        GLuint shaderId = LoadShaders(dependencies[0].c_str(), dependencies[1].c_str());
        return std::shared_ptr<ShaderProgram>(new ShaderProgram(shaderId));
    }
};
