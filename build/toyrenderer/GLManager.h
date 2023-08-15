#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include "Camera.h"
#include "Texture.h" 
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Drawable.h"
#include "Pass.h"
#include "DirectLight.h"

//glViewport(0, 0, m_width / 2, m_height / 2); draw how much screen
//texture(m_width/2,m_height/2); record how much screen into texture
//glViewport(0, 0, m_width / 2, m_height / 2);->texture(m_width,m_height);->glViewport(0, 0, m_width / 2, m_height / 2);1/16 screen display
//glViewport(0, 0, m_width / 2, m_height / 2);->texture(m_width/2,m_height/2);->glViewport(0, 0, m_width / 2, m_height / 2);1/4 screen display
const enum ShaderType
{
	SURFACE_SHADER,
	POST_SHADER,
	SHADOW_SHADER
};
class GLManager
{
private:
	GLManager();
	GLManager(const GLManager&);
	const GLManager& operator=(const GLManager&);
protected:
	GLFWwindow* m_window;
	unsigned int m_width;
	unsigned int m_height;
	
	bool m_initialized;

	PersCamera m_camera;
	OrthoCamera m_ltCamera;
	DirectLight m_dirLight;

	float lastFrame;
	float dT;

	std::vector<std::unique_ptr<ShaderProgram>> m_shaders;
	std::vector<std::unique_ptr<Texture>> m_meshTextures;
	std::vector<std::unique_ptr<Drawable>> m_meshes;
	std::vector<std::unique_ptr<FrameBuffer>> m_framebuffers;
	std::vector<std::unique_ptr<Pass>> m_passes;
	std::unordered_map<std::string, unsigned int> m_loadedTextures;

	void displayError()const;
	void resize(unsigned int width, unsigned int height);
	void setupPass();
	int initializeGL();
	void processInput();
	void paintGL();

	void setupCSM(bool debug);
	void setupSSR();
	void setupSSRHiz();
	void setupTAA();
public:
	static GLManager& getInstance() {
		static GLManager myGL;
		return myGL;
	}
	void run();
	void updateShaderUnif();//update shader uniform u_time, u_ltDir, u_ltViewProj
	ShaderProgram* addShader(const char* vertFile, const char* fragFile, ShaderType type);
	Mesh* addMesh(const char* objFile);
	Screen* addScreen();
	Plane* addPlane();
	Cube* addCube();
	Texture* addTexture(const char* texFile);
	FrameBuffer* addFrameBuffer(unsigned int width, unsigned int height, TextureType type = COLOR);
	FrameBuffer* addFrameBuffer(unsigned int width, unsigned int height, const std::vector<TextureType>& outputTex);
	GBuffer* addGBuffer(unsigned int width, unsigned int height);
	Pass* addPass(const Camera*, Drawable*, ShaderProgram*, const std::vector<TextureInfo>&,
		FrameBuffer* frameBuf = nullptr, bool forceClear = false);
	HizPass* addHizPass(Screen*, ShaderProgram*, GBuffer* frameBuf);
	TAAPass* addTAAPass(Screen*, ShaderProgram*, ShaderProgram*, const std::vector<TextureInfo>&,FrameBuffer* frameBuf=nullptr);
	void debugLog()const;
	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	friend void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};

