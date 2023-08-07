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
	VIEW_DEPTH,//z in view space
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
	bool m_mipmap;
	unsigned int m_bufId;
	TextureType m_type;
	void createEmpty()const;//create 2D texture at generated buffer m_bufId
public:
	Texture(const char* imgFile);
	Texture(unsigned int width, unsigned int height, TextureType type = COLOR, bool mipmap = false);
	virtual ~Texture();
	void useTexture(unsigned int slot = 0);
	void resize(unsigned int width, unsigned int height);
	unsigned int getWidth()const;
	unsigned int getHeight()const;
	TextureType getType()const;
	unsigned int getBuf()const;
	friend class FrameBuffer;
};

