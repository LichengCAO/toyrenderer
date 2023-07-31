#include "ShaderProgram.h"
PhongShader::PhongShader(const char* vertFile, const char* fragFile):ShaderProgram(vertFile, fragFile) {
	addAttrib("vs_pos");
	addAttrib("vs_norm");
    addAttrib("vs_uv");
	addUniform("u_viewProj");
	addUniform("u_model");
    addUniform("u_modelInvTr");
    addUniform("u_cameraPos");
	addUniform("u_ltDir");
    addUniform("u_texture");
	setUnifVec3("u_ltDir", glm::normalize(glm::vec3(1, -1, -1)));
	std::cout << std::endl;
}