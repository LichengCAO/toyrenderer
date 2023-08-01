#include "FrameBuffer.h"
FrameBuffer::FrameBuffer(unsigned int width, unsigned int height, unsigned int texNum)
	: m_width(width), m_height(height), m_FBO(0), m_depthRBO(0),
	//m_outputTex(std::vector<std::unique_ptr<Texture>>(texNum,std::make_unique<Texture>(m_width,m_height))), invalid
	m_generateFBO(false), m_generateRBO(false)
{
	std::cout << "generate framebuffer: " << m_width << "x" << m_height << std::endl;
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	
	//output texture
	GLenum drawBuffers[] = { 
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6,
		GL_COLOR_ATTACHMENT7
	};
	for (int i = 0;i < texNum;++i) {
		m_outputTex.push_back(std::make_unique<Texture>(m_width, m_height));
		auto&& tex = m_outputTex[i];
		glFramebufferTexture(GL_FRAMEBUFFER, drawBuffers[i], tex->m_bufId, 0);
	}

	//depth buffer
	bool renderDepthToTex = (texNum == 0);
	if (renderDepthToTex) {
		m_outputTex.push_back(std::make_unique<Texture>(m_width, m_height, true));
		auto&& depthTex = m_outputTex[0];
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex->m_bufId, 0);
	}
	else {
		glGenRenderbuffers(1, &m_depthRBO);
		m_generateRBO = true;
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
	}


	// Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0,
	// which we previously set to m_renderedTexture
	if(texNum>0)glDrawBuffers(texNum, drawBuffers);
	else {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	m_generateFBO = true;
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		m_generateFBO = false;
		std::cout << "Frame buffer did not initialize correctly..." << std::endl;
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL error " << error << ": ";
			const char* e =
				error == GL_INVALID_OPERATION ? "GL_INVALID_OPERATION" :
				error == GL_INVALID_ENUM ? "GL_INVALID_ENUM" :
				error == GL_INVALID_VALUE ? "GL_INVALID_VALUE" :
				error == GL_INVALID_INDEX ? "GL_INVALID_INDEX" :
				error == GL_INVALID_OPERATION ? "GL_INVALID_OPERATION" : "OTHER_ERROR";
			std::cerr << e << std::endl;
		}
	}
	useDefaultBuffer();
	std::cout << std::endl;
}
FrameBuffer::~FrameBuffer() {
	if (m_generateFBO) {
		m_generateFBO = false;
		glDeleteFramebuffers(1, &m_FBO);
		
	}
	if (m_generateRBO) {
		m_generateRBO = false;
		glDeleteRenderbuffers(1, &m_depthRBO);
	}
}

void FrameBuffer::useDefaultBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
}
void FrameBuffer::clearBuffer() {
	glClearColor(0.f, 0.0f, 0.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void FrameBuffer::resize(unsigned int width, unsigned int height) {
	m_width = width;
	m_height = height;
	for (auto&& tex : m_outputTex) {
		tex->resize(width, height);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
}
//https://blog.csdn.net/defence006/article/details/73550551
void FrameBuffer::useBuffer() {
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glEnable(GL_DEPTH_TEST);
	bool isDepthBuffer = !m_generateRBO;
	if (isDepthBuffer)glCullFace(GL_FRONT);
	else glCullFace(GL_BACK);
}
Texture* FrameBuffer::getOutputTex(unsigned int i) {
	return m_outputTex[i].get();
}