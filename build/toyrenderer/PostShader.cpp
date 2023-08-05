#include "ShaderProgram.h"
PostShader::PostShader(const char* vertFile, const char* fragFile)
	:ShaderProgram(vertFile, fragFile)
{
	addAttrib("vs_pos");
	addAttrib("vs_uv");
	addUniform("u_texture");
	addUniform("u_depth");
	addUniform("u_directLt");
	addUniform("u_norm");
	addUniform("u_pos");
	addUniform("u_albedo");
	addUniform("u_viewProj");
	addUniform("u_ltDir");
	std::cout << std::endl;
}