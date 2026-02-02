#include <glad/gl.h> 
#include <string>
#include <vector>
#include <span>

namespace Renderer
{
	void InitializeRenderedObject(GLuint& vertexArrayObject,GLuint& vertexBufferObject, std::span<const GLfloat> vertexPosition);
	GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
	GLuint CompileShader(GLuint type, const std::string& shaderSource);
	void ClearScreen();
	void RenderObject(GLuint graphicsPipelineShaderProgram, GLuint vertexBufferObject, GLuint vertexArrayObject);
}