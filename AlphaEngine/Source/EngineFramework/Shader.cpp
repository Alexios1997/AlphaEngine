
#include "EngineFrameWork/Shader.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "EngineFrameWork/Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace AlphaEngine
{

	// Getting the path of our shader, and checking if found
	Shader::Shader(const std::string& vertSrc, const std::string& fragSrc, const std::string& path) : m_FilePath(path), m_RendererID(0)
	{
		m_RendererID = CreateShader(vertSrc, fragSrc);

		if (m_RendererID == 0) {
			Logger::Err("Shader Compilation failed: " + path);
		}
		else {
			Logger::Log("Shader Async-Compiled! : " + path + " ID: " + std::to_string(m_RendererID));
		}

		uint32_t blockIndex = glGetUniformBlockIndex(m_RendererID, "CameraData");
		if (blockIndex != GL_INVALID_INDEX) {
			glUniformBlockBinding(m_RendererID, blockIndex, 0);
		}

		PreCacheUniforms();
	}

	Shader::~Shader() { glDeleteProgram(m_RendererID); }

	ShaderProgramSource Shader::ParseShader(const std::string& source)
	{
		std::stringstream ss(source);
		std::string line;
		std::stringstream shaders[2];

		enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };
		ShaderType type = ShaderType::NONE;

		while (std::getline(ss, line)) {
			if (line.find("#shader") != std::string::npos) {
				if (line.find("vertex") != std::string::npos)
					type = ShaderType::VERTEX;
				else if (line.find("fragment") != std::string::npos)
					type = ShaderType::FRAGMENT;
			}
			else if (type != ShaderType::NONE) {
				shaders[(int)type] << line << '\n';
			}
		}

		return { shaders[0].str(), shaders[1].str() };
	}

	uint32_t Shader::CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
	{
		uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexSource);
		uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

		if (vs == 0 || fs == 0) return 0;

		// Create the program if there is both vs and fs correclty read
		uint32_t programID = glCreateProgram();
		glAttachShader(programID, vs);
		glAttachShader(programID, fs);
		glLinkProgram(programID);

		int success;
		glGetProgramiv(programID, GL_LINK_STATUS, &success);

		if (!success)
		{
			char infoLog[512];
			glGetProgramInfoLog(programID, 512, NULL, infoLog);
			Logger::Err("Shader Linking Error: ");

			// Delete Everything
			glDeleteProgram(programID);
			glDeleteShader(vs);
			glDeleteShader(fs);

			return 0;
		}

		// Clean Up Everything
		glDetachShader(programID, vs);
		glDetachShader(programID, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);

		return programID;
	}

	uint32_t Shader::CompileShader(uint32_t shaderType, const std::string& specificShaderSource)
	{
		uint32_t specificShaderId = glCreateShader(shaderType);
		const char* currentSource = specificShaderSource.c_str();
		glShaderSource(specificShaderId, 1, &currentSource, nullptr);
		glCompileShader(specificShaderId);

		// Check The compilation | success/false
		GLint result;
		glGetShaderiv(specificShaderId, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE)
		{
			char infoLog[512];
			glGetShaderInfoLog(specificShaderId, 512, NULL, infoLog);

			Logger::Err("Failed to compile " +
				std::string(shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") + " shader!");

			glDeleteShader(specificShaderId);
			return 0;
		}

		return specificShaderId;
	}

	void Shader::PreCacheUniforms()
	{
		m_Uniforms.modelLoc = glGetUniformLocation(m_RendererID, "u_Model");
		m_Uniforms.viewProjLoc = glGetUniformLocation(m_RendererID, "u_ViewProjection");
		m_Uniforms.textureLoc = glGetUniformLocation(m_RendererID, "u_Texture");
		m_Uniforms.lightDirLoc = glGetUniformLocation(m_RendererID, "u_LightDir");
		m_Uniforms.viewPosLoc = glGetUniformLocation(m_RendererID, "u_ViewPos");
	}

	

	
	std::string Shader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		return result;
	}



	Shader::Shader(Shader&& other) noexcept
	{
		m_RendererID = std::move(other.m_RendererID);
		other.m_RendererID = 0;
	}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		if (this != &other) {
			glDeleteProgram(m_RendererID);

			m_RendererID = other.m_RendererID;
			other.m_RendererID = 0;
		}

		return *this;
	}

	void Shader::Use() const
	{
		glUseProgram(m_RendererID);
	}

	void Shader::StopUsing() const
	{
		glUseProgram(0);
	}

}