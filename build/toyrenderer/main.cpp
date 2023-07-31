#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "GLManager.h"

void displayError() {
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

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)glfwSetWindowShouldClose(window, true);
}
void initializeGL() {
	// Set a few settings/modes in OpenGL rendering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	displayError();
}
int main()
{
	auto& myGL = GLManager::getInstance();
	//int mesh = myGL.loadMesh("E:/GitStorage/openGL/obj/wahoo.obj");
	//int shader = myGL.addShader("E:/GitStorage/openGL/glsl/phong.vert.glsl", "E:/GitStorage/openGL/glsl/phong.frag.glsl", PHONG_SHADER);
	//int shadowCaster = myGL.addShader("E:/GitStorage/openGL/glsl/shadow.vert.glsl", "E:/GitStorage/openGL/glsl/shadow.frag.glsl", SHADOW_SHADER);
	//int postShader = myGL.addShader("E:/GitStorage/openGL/glsl/simple_post.vert.glsl", "E:/GitStorage/openGL/glsl/simple_post.frag.glsl", SIMPLE_POST_SHADER);
	//int tex = myGL.loadTexture("E:/GitStorage/openGL/texture/wahoo.bmp");
	//int screen = myGL.loadScreen();
	//int outTex = myGL.loadOutputTexture();
	//int frameBuf = myGL.loadFrameBuffer({ outTex });

	//std::vector<TextureInfo> texInfos = { /*{"u_texture",tex}*/ };
	//myGL.assignPass(mesh, shadowCaster, texInfos, frameBuf);

	//texInfos = { {"u_texture",outTex} };
	//myGL.assignPass(screen, postShader, texInfos);

	myGL.run();
	return 0;
}
