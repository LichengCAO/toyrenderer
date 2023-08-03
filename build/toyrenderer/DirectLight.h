#pragma once
#include "glm_includes.h"
#include "Pass.h"
#include <vector>
#include <memory>
#include "ShaderProgram.h"
#include "Listener.h"
//https://zhuanlan.zhihu.com/p/379042993
struct SphereBound {
	glm::vec3 center;
	float radius;
};
class DirectLight
{
protected:
	std::vector<std::unique_ptr<OrthoCamera>> m_ltCamera;
	std::vector<SphereBound> m_sphereBound;
	std::vector<float> m_splitDist; // zs
	const PersCamera* m_viewCamera;
	glm::vec3 m_ltDir;
	DirectLight(const DirectLight&);
	DirectLight& operator=(const DirectLight&);
	void calSplitDist();
	void calSphereBound();
	void setLightCameraWithBound();
public:
	DirectLight(const PersCamera* viewCamera, unsigned int casLevel = 4, const glm::vec3& ltDir = glm::vec3(0.333922, -0.46793, -0.818253));
	void updateLightCamera();
	OrthoCamera* getLightCamera(int level);
	SphereBound getSphereBound(int level);
	const glm::vec3& getLightDir()const;
	void setLightDir(const glm::vec3& ltDir);
};

