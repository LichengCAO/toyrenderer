#pragma once

#include "glm_includes.h"
#include<unordered_map>
#include<string>
#include <glad/glad.h>
#include <exception>
#include <fstream>
#include <sstream>
#include <iostream>
//#include "Drawable.h" recursion
class ShaderProgram
{
protected:
	void printShaderInfoLog(int shader)const;
	void printLinkInfoLog(int prog)const;
	void printUnifErrorLog(const std::string& name)const;
	int vertShader;
	int fragShader;
	int prog;
public:
	bool debug = false;
	std::unordered_map<std::string, int> m_attribs;
	std::unordered_map<std::string, int> m_unifs;
	ShaderProgram(const char* vertFile, const char* fragFile);
	virtual ~ShaderProgram();
	void useMe() const{
		glUseProgram(prog);
	}
	void addAttrib(const char* name);
	int  findAttrib(const std::string& name)const;
	void addUniform(const char* name);
	void setUnifMat4(const std::string& name, const glm::mat4& m)const;
	void setUnifMat3(const std::string& name, const glm::mat3& m)const;
	void setUnifVec2(const std::string& name, const glm::vec2& v)const;
	void setUnifVec3(const std::string& name, const glm::vec3& v)const;
	void setUnifFloat(const std::string& name, float f)const;
	void setUnifInt(const std::string& name, int i)const;
	void setUnifBool(const std::string& name, bool i)const;
	void debugLog()const;
};
class PostShader :
	public ShaderProgram
{
public:
	PostShader(const char* vertFile, const char* fragFile);
};
class SurfaceShader :
	public ShaderProgram
{
public:
	SurfaceShader(const char* vertFile, const char* fragFile);
};
class ShadowShader :
	public ShaderProgram
{
public:
	ShadowShader(const char* vertFile, const char* fragFile);
};
