#include "Drawable.h"
Drawable::Drawable()
	:m_drawMode(GL_TRIANGLES),m_idxBufSize(-1),m_idxBuf(0)
{
	glGenVertexArrays(1, &VAO);
	std::cout << "initialized a VAO, DONT FORGET to invoke create()!" << std::endl;
}
Drawable::~Drawable() {
	destroy();
	glDeleteVertexArrays(1, &VAO);
}
void Drawable::destroy() {
	useMe();
	for (auto& bufId : m_attribs) {
		glDeleteBuffers(1, &bufId.second);
	}
	m_attribs.clear();
	if (m_idxBufSize != -1) {
		glDeleteBuffers(1, &m_idxBuf);
	}
	m_idxBufSize = -1;
}
void Drawable::useMe()const {
	glBindVertexArray(VAO);
}
bool Drawable::useIdx()const {
	bool success = (m_idxBufSize != -1);
	if (success) {
		useMe();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuf);
	}
	return success;
}
bool Drawable::useAttrib(const std::string& name) const{
	bool success = (m_attribs.find(name) != m_attribs.end());
	if (success) {
		useMe();
		glBindBuffer(GL_ARRAY_BUFFER, m_attribs.at(name));
	}
	return success;
}
unsigned int& Drawable::addAttrib(const std::string& name) {
	useMe();
	auto& bufId = m_attribs[name];
	glGenBuffers(1, &bufId);
	return bufId;
}
void Drawable::debugLog()const {
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
void Drawable::fillAttrCheck(ShaderProgram* shader)const {
	for (auto& p : shader->m_attribs) {
		const std::string& name = p.first;
		int id = p.second;
		bool shaderHasAttr = (id != -1);
		bool thisHasAttr = (m_attribs.find(name) != m_attribs.end());
		if (shaderHasAttr && !thisHasAttr) {
			std::cout << "Object doesn't have " << name << std::endl;
		}
	}
}

void Drawable3D::setRotation(float x, float y, float z) {
	m_transformation.setRotation(x, y, z);
}
void Drawable3D::setPosition(float x, float y, float z) {
	m_transformation.setPosition(x, y, z);
}
void Drawable3D::setScale(float x, float y, float z) {
	m_transformation.setScale(x, y, z);
}
void Drawable3D::setRotation(const glm::vec3& i) {
	m_transformation.setRotation(i);
}
void Drawable3D::setPosition(const glm::vec3& i) {
	m_transformation.setPosition(i);
}
void Drawable3D::setScale(const glm::vec3& i) {
	m_transformation.setScale(i);
}
glm::mat4 Drawable3D::getModel()const {
	return m_transformation.getModel();
}
glm::mat3 Drawable3D::getModelInvTr()const {
	return m_transformation.getModelInvTr();
}