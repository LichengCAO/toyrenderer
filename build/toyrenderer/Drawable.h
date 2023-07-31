#pragma once
#include "glm_includes.h"
#include<unordered_map>
#include<string>
#include <glad/glad.h>
#include <exception>
#include <fstream>
#include <sstream>
#include <iostream>
#include "ShaderProgram.h"
#include "Transform.h"
#include <tinyobj/tiny_obj_loader.h>
class Drawable
{
protected:
	unsigned int& addAttrib(const std::string& name);
	unsigned int m_drawMode;
	int m_idxBufSize;
	unsigned int VAO;
	unsigned int m_idxBuf;
	virtual void create() = 0;
	virtual void destroy();
public:
	std::unordered_map<std::string, unsigned int> m_attribs;
	Drawable();
	virtual ~Drawable();
	virtual void draw(ShaderProgram* shader) = 0;
	void useMe()const;
	bool useIdx()const;
	bool useAttrib(const std::string& name)const;
	unsigned int drawMode()const { return m_drawMode; };
	int idBufferSize()const { return m_idxBufSize; };
	void debugLog()const;
};

class Screen :
	public Drawable
{
protected:
	virtual void create()override;
public:
	Screen();
	virtual void draw(ShaderProgram* shader)override;
};

class Plane :
	public Drawable, public Transform
{
protected:
	virtual void create()override;
public:
	Plane();
	virtual void draw(ShaderProgram* shader)override;
};

class Mesh :
	public Transform, public Drawable
{
protected:
	std::vector<unsigned int> glIndices;
	std::vector<glm::vec4> glPos;
	std::vector<glm::vec3> glNor;
	std::vector<glm::vec2> glUV;
	void create()override;
public:
	Mesh(const char* objFile);
	void draw(ShaderProgram* shader)override;
};