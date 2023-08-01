#include "Texture.h"
//https://blog.csdn.net/artisans/article/details/76695614

Texture::Texture(const char* imgFile)
	: m_imgFile(imgFile),m_width(0),m_height(0),m_generated(false),m_bufId(0)
{
	glGenTextures(1, &m_bufId);
	m_generated = true;
	glBindTexture(GL_TEXTURE_2D, m_bufId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (m_imgFile != "") {
		int nrChannels;
		stbi_set_flip_vertically_on_load(true);
		std::cout << "load texture from: " << m_imgFile << std::endl;
		unsigned char* data = stbi_load(m_imgFile.c_str(), &m_width, &m_height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}
	else {
		std::cout << "create empty texture: " << m_width << "x" << m_height << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
}
Texture::Texture(unsigned int width,unsigned int height, bool depthTex)
	: m_imgFile(""), m_width(width), m_height(height), m_generated(false), m_bufId(0)
{
	glGenTextures(1, &m_bufId);
	m_generated = true;
	glBindTexture(GL_TEXTURE_2D, m_bufId);
	if (!depthTex) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		std::cout << "create empty texture: " << m_width << "x" << m_height << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		std::cout << "create depth texture: " << m_width << "x" << m_height << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
	}
	
}
Texture::~Texture() {
	if(m_generated)glDeleteTextures(1, &m_bufId);
	m_generated = false;
}
void Texture::useTexture(unsigned int slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_bufId);
}
void Texture::resize(unsigned int width, unsigned int height) {
	if (m_imgFile != "") {
		std::cout << "Cannot resize a loaded texture" << std::endl;
	}
	else {
		m_width = width;
		m_height = height;
		glBindTexture(GL_TEXTURE_2D, m_bufId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
	}
}
unsigned int Texture::getWidth()const {
	return m_width;
}
unsigned int Texture::getHeight()const {
	return m_height;
}