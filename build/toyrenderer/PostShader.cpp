#include "ShaderProgram.h"
PostShader::PostShader(const char* vertFile, const char* fragFile)
	:ShaderProgram(vertFile, fragFile)
{
	addAttrib("vs_pos");
	addAttrib("vs_uv");
	addUniform("u_texture");
	std::cout << std::endl;
}