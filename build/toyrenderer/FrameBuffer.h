#pragma once
#include <stb_image/stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <memory>
#include "Texture.h"
class GLManager;
class HizPass;
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
	FrameBuffer();
	FrameBuffer(unsigned int width,unsigned int height,TextureType type = COLOR);
	FrameBuffer(unsigned int width, unsigned int height, const std::vector<TextureType>& outputTex);
	virtual ~FrameBuffer();
	static void renderDefaultBuffer();
	static void clearBuffer();
	static unsigned int getDefaultWidth();
	static unsigned int getDefaultHeight();
	unsigned int getWidth()const;
	unsigned int getHeight()const;
	virtual void renderBuffer();
	virtual void resize(unsigned int width, unsigned int height);
	virtual Texture* getOutputTex(unsigned int i=0);
	friend class GLManager;
};

//当子类用保护继承或者私有继承的方式继承父类时，编译器就不允许父类指针指向子类对象了。
class GBuffer
	:public FrameBuffer
{
private:
	void renderDepthBuffer(unsigned int mipmap);
	bool m_hizTexture;
public:
	GBuffer(unsigned int width, unsigned int height);
	virtual Texture* getOutputTex(unsigned int i = 0)override;
	virtual void renderBuffer()override;
	Texture* getDirectLight();
	Texture* getNormal();
	Texture* getPosition();
	Texture* getViewDepth();
	Texture* getAlbedo();
	friend class HizPass;
};

