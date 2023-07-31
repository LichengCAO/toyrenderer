#include "ShaderProgram.h"
ShadowShader::ShadowShader(const char* vertFile, const char* fragFile):
	ShaderProgram(vertFile,fragFile)
{
	addAttrib("vs_pos");
	addUniform("u_model");
	addUniform("u_viewProj");
}