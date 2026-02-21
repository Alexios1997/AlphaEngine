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

    struct StandardShaderUniforms {
        int modelLoc = -1;
        int viewProjLoc = -1;
        int textureLoc = -1;
        int lightDirLoc = -1;
        int viewPosLoc = -1;
    };

	class Shader
	{
    public:
        Shader(const std::string& vertSrc, const std::string& fragSrc, const std::string& path);
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
        static ShaderProgramSource ParseShader(const std::string& source);
        static std::string ReadFile(const std::string& filepath);
        uint32_t CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
        uint32_t CompileShader(uint32_t shaderType, const std::string& specificShaderSource);

        const StandardShaderUniforms& GetUniforms() const { return m_Uniforms; }

    private:
        uint32_t m_RendererID;
        std::string m_FilePath;
        StandardShaderUniforms m_Uniforms;
        void PreCacheUniforms();
	};
}
