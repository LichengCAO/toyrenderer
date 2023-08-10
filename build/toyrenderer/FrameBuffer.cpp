#include "FrameBuffer.h"
unsigned int FrameBuffer::u_height;
unsigned int FrameBuffer::u_width;
const GLenum BUFFER_ATTACHMENT[] = {
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7
};
FrameBuffer::FrameBuffer()
	: m_width(0), m_height(0), m_FBO(0), m_depthRBO(0), m_generateFBO(false), m_generateRBO(false)
{}
FrameBuffer::FrameBuffer(unsigned int width, unsigned int height, TextureType type)
	:FrameBuffer(width, height, std::vector<TextureType>{type})
{}
FrameBuffer::FrameBuffer(unsigned int width, unsigned int height, const std::vector<TextureType>& outputTex)
	: m_width(width), m_height(height), m_FBO(0), m_depthRBO(0), m_generateFBO(false), m_generateRBO(false)
{
	std::cout << "generate framebuffer: " << m_width << "x" << m_height << std::endl;
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	//output texture
	bool hasDepthTex = false;
	int attachId = 0;
	for (auto type : outputTex) {
		if (type == DEPTH) {
			std::unique_ptr<Texture> uPtr = std::make_unique<Texture>(m_width, m_height, DEPTH);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, uPtr->m_bufId, 0);
			m_outputTex.push_back(std::move(uPtr));
			hasDepthTex = true;
		}
		else {
			std::unique_ptr<Texture> uPtr = std::make_unique<Texture>(m_width, m_height, type);
			glFramebufferTexture(GL_FRAMEBUFFER, BUFFER_ATTACHMENT[attachId], uPtr->m_bufId, 0);
			m_outputTex.push_back(std::move(uPtr));
			++attachId;
		}
	}
	
	//depth buffer
	if (!hasDepthTex) {
		glGenRenderbuffers(1, &m_depthRBO);
		m_generateRBO = true;
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
	}

	// Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0,
	// which we previously set to m_renderedTexture
	if (attachId > 0)glDrawBuffers(attachId, BUFFER_ATTACHMENT);
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
	renderDefaultBuffer();
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

void FrameBuffer::renderDefaultBuffer() {
	glViewport(0, 0, u_width, u_height);
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
	if (m_generateRBO) {
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	}
}

//https://blog.csdn.net/defence006/article/details/73550551
void FrameBuffer::renderBuffer() {
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glEnable(GL_DEPTH_TEST);
	bool isDepthBuffer = !m_generateRBO && m_outputTex.size()==1;
	if (isDepthBuffer)glCullFace(GL_FRONT);
	else glCullFace(GL_BACK);
}
Texture* FrameBuffer::getOutputTex(unsigned int i) {
	return m_outputTex[i].get();
}
unsigned int FrameBuffer::getDefaultWidth() {
	return u_width;
}
unsigned int FrameBuffer::getDefaultHeight() {
	return u_height;
}
unsigned int FrameBuffer::getWidth()const {
	return m_width;
}
unsigned int FrameBuffer::getHeight()const {
	return m_height;
}

GBuffer::GBuffer(unsigned int width, unsigned int height)
{
	m_width = width;
	m_height = height;
	std::cout << "generate GBuffer: " << m_width << "x" << m_height << std::endl;
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	m_generateFBO = true;

	//output texture

	bool hasDepthTex = false;
	std::vector<TextureType> outputTex{HDR,NORMAL,POSITION,COLOR,VIEW_DEPTH};
	for (int i = 0;i < 4;++i) {
		std::unique_ptr<Texture> uPtr = std::make_unique<Texture>(m_width, m_height, outputTex[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, BUFFER_ATTACHMENT[i], uPtr->getBuf(), 0);
		m_outputTex.push_back(std::move(uPtr));
	}
	std::unique_ptr<Texture> uPtr = std::make_unique<Texture>(m_width, m_height, VIEW_DEPTH,true);
	glFramebufferTexture(GL_FRAMEBUFFER, BUFFER_ATTACHMENT[4], uPtr->getBuf(), 0);
	m_outputTex.push_back(std::move(uPtr));

	//depth buffer
	glGenRenderbuffers(1, &m_depthRBO);
	m_generateRBO = true;
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);

	// Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0,
	// which we previously set to m_renderedTexture
	glDrawBuffers(5, BUFFER_ATTACHMENT);
	
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
	renderDefaultBuffer();
	std::cout << std::endl;
}
Texture* GBuffer::getOutputTex(unsigned int i) {
	std::cout << "this is a GBuffer use other function to get output texture" << std::endl;
	return nullptr;
}
void GBuffer::renderDepthBuffer(unsigned int mipmap) {
	unsigned int minSize = 1;
	unsigned int width = std::max(minSize, m_width >> mipmap);
	unsigned int height = std::max(minSize, m_height >> mipmap);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	m_hizTexture = true;
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, getViewDepth()->getBuf(), mipmap);
	glDrawBuffers(1, BUFFER_ATTACHMENT);//important, tell buffer to render to only this texture
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
}
void GBuffer::renderBuffer() {
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glEnable(GL_DEPTH_TEST);
	if (m_hizTexture) {
		//rebind textures
		int attachId = 0;
		for (auto&& tex : m_outputTex) {
			glFramebufferTexture(GL_FRAMEBUFFER, BUFFER_ATTACHMENT[attachId], tex->getBuf(), 0);
			++attachId;
		}
		m_hizTexture = false;
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
		glDrawBuffers(5, BUFFER_ATTACHMENT);//important, tell buffer to render to only this texture
	}
	glCullFace(GL_BACK);
}
Texture* GBuffer::getDirectLight() {
	return FrameBuffer::getOutputTex(0);
}
Texture* GBuffer::getNormal() {
	return FrameBuffer::getOutputTex(1);
}
Texture* GBuffer::getPosition() {
	return FrameBuffer::getOutputTex(2);
}
Texture* GBuffer::getViewDepth() {
	return FrameBuffer::getOutputTex(4);
}
Texture* GBuffer::getAlbedo() {
	return FrameBuffer::getOutputTex(3);
}