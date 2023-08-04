#include "Texture.h"
//https://blog.csdn.net/artisans/article/details/76695614
Texture::Texture(const char* imgFile)
	: m_imgFile(imgFile),m_width(0),m_height(0),m_generated(false),m_bufId(0),m_type(COLOR)
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
}
Texture::Texture(unsigned int width,unsigned int height, TextureType type)
	: m_imgFile(""), m_width(width), m_height(height), m_generated(false), m_bufId(0),m_type(type)
{
	glGenTextures(1, &m_bufId);
	m_generated = true;
	glBindTexture(GL_TEXTURE_2D, m_bufId);
	std::cout << "create empty texture: " << m_width << "x" << m_height;
	switch (type) {
	case COLOR:
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		std::cout << " to store color(RGBA)" << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
		break;
	}
	case NORMAL:
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		std::cout << " to store normal" << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, (void*)0);
		break;
	}
	case POSITION:
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		std::cout << " to store position" << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, (void*)0);
		break;
	}
	case DEPTH:
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		std::cout << " to store depth" << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
		break;
	}
	default:
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		std::cout << " to store color(RGBA)" << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
		//glGenerateMipmap(GL_TEXTURE_2D);
		break;
	}
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
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
		switch (m_type) {
		case COLOR:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
			break;
		}
		case NORMAL:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, (void*)0);
			break;
		}
		case POSITION:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, (void*)0);
			break;
		}
		case DEPTH:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
			break;
		}
		default:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
			break;
		}
		}
	}
}
unsigned int Texture::getWidth()const {
	return m_width;
}
unsigned int Texture::getHeight()const {
	return m_height;
}