#include "ShaderProgram.h"
PhongShader::PhongShader(const char* vertFile, const char* fragFile):ShaderProgram(vertFile, fragFile) {
	addAttrib("vs_pos");
	addAttrib("vs_norm");
    addAttrib("vs_uv");
	
	addUniform("u_model");
	addUniform("u_viewProj");
    addUniform("u_modelInvTr");
	
	addUniform("u_ltViewProj");
	addUniform("u_ltDir");
	
	addUniform("u_cameraPos");
    
	addUniform("u_texture");
	addUniform("u_depth");
	
	setUnifVec3("u_ltDir", glm::normalize(glm::vec3(1, -1, -1)));
	std::cout << std::endl;
}