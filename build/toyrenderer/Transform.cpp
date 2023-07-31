#include "Transform.h"
void Transform::computeModelMat() {
	glm::mat4 scale = glm::scale(glm::mat4(), m_scale);
	glm::mat4 rotateX = glm::rotate(glm::mat4(), m_rot.x, glm::vec3(1, 0, 0));
	glm::mat4 rotateY = glm::rotate(glm::mat4(), m_rot.y, glm::vec3(0, 1, 0));
	glm::mat4 rotateZ = glm::rotate(glm::mat4(), m_rot.z, glm::vec3(0, 0, 1));
	glm::mat4 trans = glm::translate(glm::mat4(),m_pos);
	m_modelMat = trans * rotateZ * rotateY * rotateX * scale;
	m_modelMatInvTr = glm::mat3(glm::transpose(glm::inverse(m_modelMat)));
}
void Transform::setRotation(float x, float y, float z) {
	setRotation(glm::vec3(x, y, z));
}
void Transform::setPosition(float x, float y, float z) {
	setPosition(glm::vec3(x, y, z));
}
void Transform::setScale(float x, float y, float z) {
	setScale(glm::vec3(x, y, z));
}
void Transform::setRotation(const glm::vec3& i) {
	m_rot = i;
	computeModelMat();
}
void Transform::setPosition(const glm::vec3& i) {
	m_pos = i;
	computeModelMat();
}
void Transform::setScale(const glm::vec3& i) {
	m_scale = i;
	computeModelMat();
}
glm::mat4 Transform::getModel()const {
	return m_modelMat;
}
glm::mat3 Transform::getModelInvTr()const {
	return m_modelMatInvTr;
}
Transform::Transform()
	:m_modelMat(glm::mat4()), m_modelMatInvTr(glm::mat3()),
	m_pos(glm::vec3(0.f)), m_rot(glm::vec3(0.f)), m_scale(glm::vec3(1.f))
{}

