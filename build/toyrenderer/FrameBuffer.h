#pragma once
#include <stb_image/stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <memory>
#include "Texture.h"
class GLManager;
class FrameBuffer
{
protected:
	static unsigned int u_width;//default framebuffer width
	static unsigned int u_height;//default framebuffer height
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_FBO;
	unsigned int m_depthRBO;
	std::vector<std::unique_ptr<Texture>> m_outputTex;
	bool m_generateFBO;
	bool m_generateRBO;
public:
	FrameBuffer(unsigned int width,unsigned int height,unsigned int texNum = 1);
	virtual ~FrameBuffer();
	static void useDefaultBuffer();
	static void clearBuffer();
	static unsigned int getDefaultWidth();
	static unsigned int getDefaultHeight();
	unsigned int getWidth()const;
	unsigned int getHeight()const;
	void useBuffer();
	void resize(unsigned int width, unsigned int height);
	Texture* getOutputTex(unsigned int i=0);
	friend class GLManager;
};

