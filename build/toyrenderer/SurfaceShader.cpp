#include "ShaderProgram.h"
SurfaceShader::SurfaceShader(const char* vertFile, const char* fragFile):ShaderProgram(vertFile, fragFile) {
	addAttrib("vs_pos");
	addAttrib("vs_norm");
    addAttrib("vs_uv");
	
	addUniform("u_model");
	addUniform("u_viewProj");
    addUniform("u_modelInvTr");
	
	addUniform("u_ltDir");
	addUniform("u_ltViewProj");
	
	addUniform("u_cameraPos");
    
	addUniform("u_texture");
	addUniform("u_depthBiasA");
	addUniform("u_depth");

	for (int i = 0;i < 4;++i) {
		std::string u_id = "[" + std::to_string(i) + "]";
		std::string u_depth = "u_depth" + u_id;
		std::string u_sphere = "u_sphere" + u_id;
		std::string u_radius = "u_radius" + u_id;
		std::string u_viewProj = "u_ltViewProj" + u_id;
		addUniform(u_depth.c_str());
		addUniform(u_sphere.c_str());
		addUniform(u_radius.c_str());
		addUniform(u_viewProj.c_str());
	}
	
	setUnifVec3("u_ltDir", glm::normalize(glm::vec3(1, -1, -1)));
	std::cout << std::endl;
}