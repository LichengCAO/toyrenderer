#include "Pass.h"
FrameBuffer* Pass::u_bufferInUse = nullptr;
Pass::Pass()
	:m_camera(nullptr), m_drawable(nullptr), m_shader(nullptr),
	m_frameBuf(nullptr), m_forceClear(false)
{}
Pass::Pass(const Camera* camera,
	Drawable* drawable, 
	ShaderProgram* shader, 
	const std::vector<TextureInfo>& texInfo,
	FrameBuffer* frameBuf,  bool forceClear)
	:m_camera(camera),m_drawable(drawable),m_shader(shader),
	m_texInfos(texInfo),
	m_frameBuf(frameBuf),m_forceClear(forceClear)
{}
bool Pass::needClearBuffer()const {
	return (m_forceClear || Pass::u_bufferInUse != m_frameBuf);
}
void Pass::run() {
	//bind to assigned buffer
	if (m_frameBuf != nullptr) {
		m_frameBuf->renderBuffer();
	}else {
		FrameBuffer::renderDefaultBuffer();
	}
	//check if we need to clear buffer
	if (needClearBuffer()) {
		FrameBuffer::clearBuffer();
	}
	//update u_bufferInUse
	Pass::u_bufferInUse = m_frameBuf;
	//set camera information for the shader
	if (m_camera != nullptr) {
		m_shader->setUnifMat4("u_viewProj", m_camera->getViewProj());
		m_shader->setUnifVec3("u_cameraPos", m_camera->eye);
		m_shader->setUnifMat4("u_view", m_camera->getView());
	}
	//set texture slot for the shader
	for (int i = 0;i < m_texInfos.size();++i) {
		TextureInfo& tex = m_texInfos[i];
		m_shader->setUnifInt(tex.name, i);
		tex.tex->useTexture(i);
	}
	//draw meshes to framebuffer assigned
	m_drawable->draw(m_shader);
}

HizPass::HizPass(Screen* screen, ShaderProgram* hiz, GBuffer* frameBuf)
	:Pass(), m_GBuffer(frameBuf)
{
	m_drawable = screen;
	m_shader = hiz;
	m_frameBuf = frameBuf;
	m_forceClear = true;
	m_texInfos = std::vector<TextureInfo>();
	m_texInfos.push_back({ "u_texture",m_GBuffer->getViewDepth() });
}
void HizPass::run() {
	//update u_bufferInUse
	Pass::u_bufferInUse = m_frameBuf;
	//set texture slot for the shader
	for (int i = 0;i < m_texInfos.size();++i) {
		TextureInfo& tex = m_texInfos[i];
		m_shader->setUnifInt(tex.name, i);
		tex.tex->useTexture(i);
	}

	unsigned int minSize = 1;
	unsigned int curWidth = m_GBuffer->getWidth();
	unsigned int curHeight = m_GBuffer->getHeight();
	//https://zhuanlan.zhihu.com/p/552432586
	int numLevels = 1 + (int)floorf(log2f(fmaxf(curWidth, curHeight)));
	for (int i = 1;i < numLevels;++i) {
		m_GBuffer->renderDepthBuffer(i);
		FrameBuffer::clearBuffer();
		m_shader->setUnifInt("u_mipLevel", i);
		m_shader->setUnifInt("u_prevWidth", curWidth);
		m_shader->setUnifInt("u_prevHeight", curHeight);
		curWidth = std::max(curWidth / 2, minSize);
		curHeight = std::max(curHeight / 2, minSize);
		m_drawable->draw(m_shader);
	}	
}

float ShadowedPass::calBiasA(const Texture* shadowTex){
	unsigned int shadowMapSize = std::max(shadowTex->getHeight(), shadowTex->getWidth());
	unsigned int frustumSize = 0;
	if (m_frameBuf == nullptr) {
		frustumSize = std::max(FrameBuffer::getDefaultWidth(), FrameBuffer::getDefaultHeight());
	}
	else {
		frustumSize = std::max(m_frameBuf->getWidth(), m_frameBuf->getHeight());
	}
	return float(frustumSize) / float(2 * shadowMapSize);
}
ShadowedPass::ShadowedPass(const Camera* camera,
	Drawable* drawable,
	ShaderProgram* shader,
	const std::vector<TextureInfo>& texInfo,
	FrameBuffer* frameBuf, bool forceClear)
	: Pass(camera,drawable,shader,texInfo,frameBuf,forceClear)
{
	std::cout << "please use addShadowMap() to load shadowmaps" << std::endl;
}
void ShadowedPass::addShadowMap(const TextureInfo& shadowMap) {
	float biasA = calBiasA(shadowMap.tex);
	m_shadowInfos.push_back({ shadowMap,biasA });
}
void ShadowedPass::run(){
	//bind to assigned buffer
	if (m_frameBuf != nullptr) {
		m_frameBuf->renderBuffer();
	}
	else {
		FrameBuffer::renderDefaultBuffer();
	}
	//check if we need to clear buffer
	if (needClearBuffer()) {
		FrameBuffer::clearBuffer();
	}
	//update u_bufferInUse
	Pass::u_bufferInUse = m_frameBuf;
	//set camera information for the shader
	if (m_camera != nullptr) {
		m_shader->setUnifMat4("u_viewProj", m_camera->getViewProj());
		m_shader->setUnifVec3("u_cameraPos", m_camera->eye);
	}
	//set texture slot for the shader
	int i = 0;
	int n = m_texInfos.size();
	for (;i < n;++i) {
		TextureInfo& tex = m_texInfos[i];
		m_shader->setUnifInt(tex.name, i);
		tex.tex->useTexture(i);
	}
	//set shadow map for the shader
	n = m_shadowInfos.size();
	for (int j = 0;j < n;++j) {
		auto& p = m_shadowInfos[j];
		TextureInfo& tex = p.first;
		m_shader->setUnifInt(tex.name, i + j);
		tex.tex->useTexture(i + j);
		float biasA = p.second;
		m_shader->setUnifFloat(tex.name + "BiasA", biasA);
	}
	//draw meshes to framebuffer assigned
	m_drawable->draw(m_shader);
}

TAAPass::TAAPass(const Camera* camera, Screen* srn, ShaderProgram* shader, ShaderProgram* simplePass, const std::vector<TextureInfo>& texInfo, FrameBuffer* outputFrame)
	:Pass(camera,srn,shader,texInfo,outputFrame,true),m_simplePassShader(simplePass)
{
	int width = m_frameBuf == nullptr ? FrameBuffer::getDefaultWidth() : m_frameBuf->getWidth();
	int height = m_frameBuf == nullptr ? FrameBuffer::getDefaultHeight() : m_frameBuf->getHeight();
	m_recordBuf.push_back(std::make_unique<FrameBuffer>(width, height));
	m_recordBuf.push_back(std::make_unique<FrameBuffer>(width, height));
}
void TAAPass::run() {
	FrameBuffer* prevFrame = nullptr;
	FrameBuffer* frameToRecord = nullptr;
	if (m_useFirstRecord) {
		frameToRecord = m_recordBuf[0].get();
		prevFrame = m_recordBuf[1].get();
	}
	else {
		frameToRecord = m_recordBuf[1].get();
		prevFrame = m_recordBuf[0].get();
	}
	frameToRecord->renderBuffer();
	m_useFirstRecord = !m_useFirstRecord;
	FrameBuffer::clearBuffer();
	//set camera information for the shader
	if (m_camera != nullptr) {
		m_shader->setUnifMat4("u_viewProj", m_camera->getViewProj());
		m_shader->setUnifVec3("u_cameraPos", m_camera->eye);
		m_shader->setUnifMat4("u_view", m_camera->getView());
		m_shader->setUnifMat4("u_prevViewProj", m_prevViewProj);
		m_prevViewProj = m_camera->getViewProj();
	}
	//set texture slot for the shader
	for (int i = 0;i < m_texInfos.size();++i) {
		TextureInfo& tex = m_texInfos[i];
		m_shader->setUnifInt(tex.name, i);
		tex.tex->useTexture(i);
		//std::cout << tex.name << std::endl;
	}
	//set prevFrame texture
	m_shader->setUnifInt("u_prevFrame", m_texInfos.size());
	prevFrame->getOutputTex()->useTexture(m_texInfos.size()); 
	//draw meshes to framebuffer assigned
	m_drawable->draw(m_shader);
	//bind to assigned buffer
	if (m_frameBuf != nullptr) {
		m_frameBuf->renderBuffer();
	}
	else {
		FrameBuffer::renderDefaultBuffer();
	}
	//check if we need to clear buffer
	if (needClearBuffer()) {
		FrameBuffer::clearBuffer();
	}
	//update u_bufferInUse
	Pass::u_bufferInUse = m_frameBuf;
	m_simplePassShader->setUnifInt("u_texture", 0);
	frameToRecord->getOutputTex()->useTexture(0);
	//draw meshes to framebuffer assigned
	m_drawable->draw(m_simplePassShader);
}