#pragma once
#include "glm_includes.h"
class Transform
{
protected:
    glm::mat4 m_modelMat;
    glm::mat3 m_modelMatInvTr;
    glm::vec3 m_pos;
    glm::vec3 m_rot;
    glm::vec3 m_scale;
    void computeModelMat();
public:
    Transform();
    void setRotation(float x, float y, float z);
    void setPosition(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setRotation(const glm::vec3& i);
    void setPosition(const glm::vec3& i);
    void setScale(const glm::vec3& i);
    glm::mat4 getModel()const;
    glm::mat3 getModelInvTr()const;
};

