#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/gl.h>

namespace AlphaEngine 
{

    struct ShaderProgramSource {
        std::string VertexSource;
        std::string FragmentSource;
    };

	class Shader
	{
    public:
        Shader(const std::string& filepath);
        ~Shader();

        // Disable Copying - We pretty much avoid Double deletion
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // Move semantics
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        void Use() const;
        void StopUsing() const;

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        uint32_t m_RendererID;
        std::string m_FilePath;


        std::string ReadFile(const std::string& filepath);
        ShaderProgramSource ParseShader(const std::string& source);
        uint32_t CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
        uint32_t CompileShader(uint32_t shaderType, const std::string& specificShaderSource);
	};
}
