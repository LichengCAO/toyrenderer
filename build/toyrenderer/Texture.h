#pragma once
#include <stb_image/stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <vector>
class FrameBuffer;
enum TextureType {
	COLOR,
	NORMAL,
	POSITION,
	DEPTH,
	HDR
};
class Texture
{
protected:	
	std::string m_imgFile;
	int m_width;
	int m_height;
	bool m_generated;
	unsigned int m_bufId;
	TextureType m_type;
public:
	Texture(const char* imgFile);
	Texture(unsigned int width, unsigned int height, TextureType type = COLOR);
	virtual ~Texture();
	void useTexture(unsigned int slot = 0);
	void resize(unsigned int width, unsigned int height);
	unsigned int getWidth()const;
	unsigned int getHeight()const;
	friend class FrameBuffer;
};

