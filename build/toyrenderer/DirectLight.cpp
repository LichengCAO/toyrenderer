#include "DirectLight.h"

std::vector<OrthoCamera> m_ltCamera;
std::vector<SphereBound> m_sphereBound;
std::vector<float> m_splitDist; // zs
const PersCamera* m_viewCamera;
DirectLight::DirectLight(const PersCamera* viewCamera, unsigned int casLevel, const glm::vec3& ltDir)
	://m_ltCamera(std::vector<OrthoCamera>(casLevel,std::make_unique<OrthoCamera>()),
	m_sphereBound(std::vector<SphereBound>(casLevel)),
	m_splitDist(std::vector<float>(casLevel + 1, 0.f)),
	m_viewCamera(viewCamera), m_ltDir(ltDir)
{
	for (int i = 0;i < casLevel;++i) {
		m_ltCamera.push_back(std::make_unique<OrthoCamera>());
	}
	std::cout << "generate light camera with " << casLevel << " levels" << std::endl;
	std::cout << "light direction: " << m_ltDir.x << ", " << m_ltDir.y << ", " << m_ltDir.z << std::endl;
	calSplitDist();
	updateLightCamera();
}

void DirectLight::calSplitDist() {
	int N = m_ltCamera.size();
	if (m_viewCamera == nullptr) {
		std::cout << "no view camera" << std::endl;
		return;
	}
	float n = m_viewCamera->near_clip;
	float f = m_viewCamera->far_clip;
	float f_n = f / n;
	for (int i = 1;i <= N;++i) {
		float ideal = n * pow(f_n, float(i) / float(N));
		float linear = n + i * (f - n) / float(N);
		m_splitDist[i] = glm::mix(ideal, linear, 0.1f);
		std::cout << m_splitDist[i] << std::endl;
	}
	m_splitDist[0] = n;
}
//sphere bounding box
void DirectLight::calSphereBound() {
	if (m_viewCamera == nullptr) {
		std::cout << "no view camera" << std::endl;
		return;
	}
	int N = m_ltCamera.size();
	float y = glm::tan(glm::radians(m_viewCamera->fovy/2.f));
	float x = y / float(m_viewCamera->height) * float(m_viewCamera->width);
	float wNorm_2 = x * x + y * y;
	for (int i = 0;i < N;++i) {
		double w1_2 = wNorm_2 * (m_splitDist[i + 1] * m_splitDist[i + 1]);
		double w2_2 = wNorm_2 * (m_splitDist[i] * m_splitDist[i]);
		double h_2 = glm::pow(m_splitDist[i + 1] - m_splitDist[i], 2);
		double r_2 = glm::pow((-w2_2 + w1_2 - h_2)/2, 2) + w1_2;
		float toSphereLen = std::sqrt(r_2 - w2_2) + m_splitDist[i];
		m_sphereBound[i].center = m_viewCamera->eye + m_viewCamera->look * toSphereLen;
		m_sphereBound[i].radius = std::sqrt(r_2);
	}
	//for (auto s : m_sphereBound) {
	//	std::cout << s.center.x << "," << s.center.y << ", " << s.center.z << std::endl;
	//}
	//std::cout << "########################################" << std::endl;
}
void DirectLight::updateLightCamera() {
	if (m_viewCamera == nullptr) {
		std::cout << "no view camera" << std::endl;
		return;
	}
	calSphereBound();
	setLightCameraWithBound();
}
OrthoCamera* DirectLight::getLightCamera(int level) {
	return m_ltCamera[level].get();
}
SphereBound DirectLight::getSphereBound(int level) {
	return m_sphereBound[level];
}

const glm::vec3& DirectLight::getLightDir()const {
	return m_ltDir;
}
void DirectLight::setLightDir(const glm::vec3& ltDir) {
	m_ltDir = ltDir;
	setLightCameraWithBound();
}
void DirectLight::setLightCameraWithBound() {
	int N = m_ltCamera.size();
	for (int i = 0;i < N;++i) {
		//make our camera fill this sphere
		float radius = m_sphereBound[i].radius;
		OrthoCamera* ltCamera = m_ltCamera[i].get();
		ltCamera->width = ltCamera->height = radius*2;
		ltCamera->far_clip = radius*5;
		ltCamera->MoveTo(m_sphereBound[i].center - m_ltDir * radius*3.f);
		ltCamera->LookAlong(m_ltDir);
	}
}