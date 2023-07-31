#include "Pass.h"
Pass::Pass(const Camera* camera,
	Drawable* drawable, 
	ShaderProgram* shader, 
	const std::vector<TextureInfo>& texInfo)
	:m_camera(camera),m_drawable(drawable),m_shader(shader),
	m_texInfos(texInfo)
{}

void Pass::run() {
	//set camera information for the shader
	//set texture slot for the shader
	//draw meshes to framebuffer assigned
	if (m_camera != nullptr) {
		m_shader->setUnifMat4("u_viewProj", m_camera->getViewProj());
		m_shader->setUnifVec3("u_cameraPos", m_camera->eye);
	}
	for (int i = 0;i < m_texInfos.size();++i) {
		TextureInfo& tex = m_texInfos[i];
		m_shader->setUnifInt(tex.name, i);
		tex.tex->useTexture(i);
	}
	m_drawable->draw(m_shader);
}