#include "GLManager.h"
#include <functional>
bool firstMouse = true;
float lastX = 400, lastY = 300;
float yaw = 0.f, pitch = 0.f;
float sensitivity = 0.3;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	std::cout << "size call back" << std::endl;
	auto& manager = GLManager::getInstance();
	manager.resize(width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	auto& manager = GLManager::getInstance();
	Camera& camera = manager.m_camera;
	
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float dx = xpos - lastX;
	float dy = lastY - ypos;
	dx *= sensitivity;
	dy *= sensitivity;
	yaw += dx;
	pitch += dy;
	if (pitch > 89.f) {
		pitch = 89.f;
	}
	if (pitch < -89.f) {
		pitch = -89.f;
	}
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera.LookAlong(front);
	lastX = xpos;
	lastY = ypos;
}

GLManager::GLManager()
	:m_window(nullptr),
	m_camera(640, 480, glm::vec3(0, 0, 12), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
	//m_camera(20,20, glm::vec3(0, 0, 12), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
	m_ltCamera(20, 20, glm::vec3(0, 0, 12), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
	m_dirLight(&m_camera),
	m_width(800), m_height(600),
	m_initialized(false),
	lastFrame(0.0),dT(0.0)
{
	m_initialized = (initializeGL()==0);
}
void GLManager::displayError() const{
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
void GLManager::resize(unsigned int width, unsigned int height) {
	m_width = width;
	m_height = height;
	FrameBuffer::u_width = width;
	FrameBuffer::u_height = height;
}
ShaderProgram* GLManager::addShader(const char* vertFile, const char* fragFile, ShaderType type) {
	int shaderId = m_shaders.size();
	ShaderProgram* res = nullptr;
	//transfer of control bypasses initialization of: variable xxx
	//https://blog.csdn.net/venom_snake/article/details/106784095
	switch (type) {
	case SURFACE_SHADER:
	{
		std::unique_ptr<SurfaceShader> phongShader = std::make_unique<SurfaceShader>(vertFile, fragFile);
		res = phongShader.get();
		m_shaders.push_back(std::move(phongShader));
		break;
	}
	case POST_SHADER:
	{
		std::unique_ptr<PostShader> simplePostShader = std::make_unique<PostShader>(vertFile, fragFile);
		res = simplePostShader.get();
		m_shaders.push_back(std::move(simplePostShader));
		break;
	}
	case SHADOW_SHADER:
	{
		std::unique_ptr<ShadowShader> shadowShader = std::make_unique<ShadowShader>(vertFile, fragFile);
		res = shadowShader.get();
		m_shaders.push_back(std::move(shadowShader));
		break;
	}
	default:
		std::cout << "no such shader type" << std::endl;
	}
	return res;
}
Mesh* GLManager::addMesh(const char* objFile) {
	std::unique_ptr<Mesh> uPtr = std::make_unique<Mesh>(objFile);
	Mesh* res = uPtr.get();
	m_meshes.push_back(std::move(uPtr));
	return res;
}
Screen* GLManager::addScreen() {
	std::unique_ptr<Screen> screen = std::make_unique<Screen>();
	Screen* res = screen.get();
	m_meshes.push_back(std::move(screen));
	return res;
}
Plane* GLManager::addPlane() {
	std::unique_ptr<Plane> uPtr = std::make_unique<Plane>();
	Plane* res = uPtr.get();
	m_meshes.push_back(std::move(uPtr));
	return res;
}
Texture* GLManager::addTexture(const char* texFile) {
	bool loaded = m_loadedTextures.find(texFile) != m_loadedTextures.end();
	if (loaded) {
		std::cout << "already loaded texture: " << texFile << std::endl;
		unsigned int idx = m_loadedTextures[texFile];
		return m_meshTextures[idx].get();
	}
	std::unique_ptr<Texture> tex = std::make_unique<Texture>(texFile);
	Texture* res = tex.get();
	m_loadedTextures[texFile] = m_meshTextures.size();
	m_meshTextures.push_back(std::move(tex));
	return res;
}
FrameBuffer* GLManager::addFrameBuffer(unsigned int width, unsigned int height, unsigned int texNum) {
	std::unique_ptr<FrameBuffer> frameBuf = std::make_unique<FrameBuffer>(width, height, texNum);
	FrameBuffer* res = frameBuf.get();
	m_framebuffers.push_back(std::move(frameBuf));
	return res;
}
Pass* GLManager::addPass(const Camera* camera, Drawable* drawable, ShaderProgram* shader, const std::vector<TextureInfo>& texInfo,
	FrameBuffer* framebuffer,bool forceClear) {
	//if the pass is first pass we need to clear the buffer
	if (m_passes.size() == 0) {
		forceClear = true;
		std::cout << "Tell first pass to clear its buffer" << std::endl;
	}
	std::unique_ptr<Pass> pass = std::make_unique<Pass>(camera, drawable, shader, texInfo,framebuffer,forceClear);
	Pass* res = pass.get();
	m_passes.push_back(std::move(pass));
	return res;
}
ShadowedPass* GLManager::addShadowedPass(const Camera* camera, 
	Drawable* drawable, ShaderProgram* shader, 
	const std::vector<TextureInfo>& texInfo,
	FrameBuffer* framebuffer, bool forceClear) {
	if (m_passes.size() == 0) {
		forceClear = true;
		std::cout << "Tell first pass to clear its buffer" << std::endl;
	}
	std::unique_ptr<ShadowedPass> uPtr = std::make_unique<ShadowedPass>(camera, drawable, shader, texInfo, framebuffer, forceClear);
	ShadowedPass* res = uPtr.get();
	m_passes.push_back(std::move(uPtr));
	return res;
}
void GLManager::debugLog()const {
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

int GLManager::initializeGL() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(m_width, m_height, "toy renderer", NULL, NULL);
	if (m_window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(m_window);
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	displayError();
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	glfwSetCursorPosCallback(m_window, mouse_callback);
	resize(m_width, m_height);
	return 0;
}
void GLManager::processInput() {
	float cameraSpeed = 0.5f; // adjust accordingly
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(m_window, true);
	}
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
		m_camera.TranslateAlongLook(cameraSpeed * dT);
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
		m_camera.TranslateAlongLook(-cameraSpeed * dT);
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		m_camera.TranslateAlongRight(-cameraSpeed * dT);
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
		m_camera.TranslateAlongRight(cameraSpeed * dT);
	}
	if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS) {
		m_camera.TranslateAlongUp(-cameraSpeed * dT);
	}
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS) {
		m_camera.TranslateAlongUp(cameraSpeed * dT);
	}
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
		std::cout <<"camera world position: (" << m_camera.eye.x << ", "<< m_camera.eye.y << "," << m_camera.eye.z << ")" << std::endl;
		std::cout << "camera look: (" << m_camera.look.x << ", " << m_camera.look.y << "," << m_camera.look.z << ")" << std::endl;
	}
}
void GLManager::run() {
	if (!m_initialized) {
		std::cout << "initialized failed" << std::endl;
		return;
	}
	float curFrame = glfwGetTime();
	dT = curFrame - lastFrame;
	lastFrame = curFrame;
	float deg = 0.0f;
	setupPass();
	while (!glfwWindowShouldClose(m_window)) {
		processInput();
		m_dirLight.updateLightCamera();
		paintGL();
	}
	glfwTerminate();
}
void GLManager::paintGL() {
	FrameBuffer::useDefaultBuffer();
	FrameBuffer::clearBuffer();
	FrameBuffer* prevBuffer = nullptr;//default buffer
	updateShaderUnif();
	for (auto&& pass : m_passes) {
		//let pass draw
		pass->run();
	}
	//FrameBuffer::useDefaultBuffer();
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}
//update shader uniform u_time, u_ltDir, u_ltViewProj
void GLManager::updateShaderUnif() {
	glm::mat4 ltViewProj = m_ltCamera.getViewProj();
	glm::vec3 ltDir = m_dirLight.getLightDir();
	for (auto&& shader : m_shaders) {
		shader->setUnifMat4("u_ltViewProj", ltViewProj);
		shader->setUnifFloat("u_time", lastFrame);
		shader->setUnifVec3("u_ltDir", ltDir);
		for (int i = 0;i < 4;++i) {
			std::string u_id = "[" + std::to_string(i) + "]";
			std::string u_sphere = "u_sphere" + u_id;
			std::string u_radius = "u_radius" + u_id;
			std::string u_viewProj = "u_ltViewProj" + u_id;
			auto sphere = m_dirLight.getSphereBound(i);
			shader->setUnifVec3(u_sphere,sphere.center);
			shader->setUnifFloat(u_radius, sphere.radius);
			shader->setUnifMat4(u_viewProj, m_dirLight.getLightCamera(i)->getViewProj());
		}
	}
}
void GLManager::setupPass() {
	Mesh* mario = addMesh("E:/GitStorage/openGL/obj/wahoo.obj");
	Screen* screen = addScreen();
	Plane* plane = addPlane();
	
	ShaderProgram* debugShader = addShader("E:/GitStorage/openGL/glsl/basic.vert.glsl", "E:/GitStorage/openGL/glsl/basic.frag.glsl", SURFACE_SHADER);
	ShaderProgram* PCSS = addShader("E:/GitStorage/openGL/glsl/PCSS.vert.glsl", "E:/GitStorage/openGL/glsl/PCSS.frag.glsl", SURFACE_SHADER);
	ShaderProgram* CSM = addShader("E:/GitStorage/openGL/glsl/CSM.vert.glsl", "E:/GitStorage/openGL/glsl/CSM.frag.glsl", SURFACE_SHADER);
	Texture* planeTex = addTexture("E:/GitStorage/openGL/texture/plane.bmp");
	Texture* marioTex = addTexture("E:/GitStorage/openGL/texture/wahoo.bmp");
	FrameBuffer* framebuffer = addFrameBuffer(m_width*2, m_height*2,0);//create depth texture with 0

	plane->setScale(glm::vec3(150.f));
	plane->setRotation(glm::vec3(-90, 0, 0));
	plane->setPosition(glm::vec3(0, -4, 0));

	//setup shadowmapping pass
	std::vector<TextureInfo> emptyTex;
	addPass(&m_camera, mario, debugShader, emptyTex);
	addPass(&m_camera, plane, debugShader, emptyTex);
	
	ShaderProgram* shadow = addShader("E:/GitStorage/openGL/glsl/shadow.vert.glsl", "E:/GitStorage/openGL/glsl/shadow.frag.glsl", SHADOW_SHADER);
	shadow = debugShader;
	unsigned int shadowTexWidth = m_width;
	unsigned int shadowTexHeight = m_width;

	FrameBuffer* shadowBuffer[4];
	for (int i = 0;i < 4;++i) {
		shadowBuffer[i] = addFrameBuffer(shadowTexWidth, shadowTexHeight, 0);
		Camera* ltCamera = m_dirLight.getLightCamera(i);
		addPass(ltCamera, mario, shadow, emptyTex, shadowBuffer[i]);
		addPass(ltCamera, plane, shadow, emptyTex, shadowBuffer[i]);
		//addPass(ltCamera, mario, debugShader, emptyTex, debugBuffer[i]);
		//addPass(ltCamera, plane, debugShader, emptyTex, debugBuffer[i]);
	}

	//setup 3d render pass
	std::vector<TextureInfo> shadowMap;
	for (int i = 0;i < 4;++i) {
		std::string u_id = "[" + std::to_string(i) + "]";
		std::string u_depth = "u_depth" + u_id;
		shadowMap.push_back({ u_depth,shadowBuffer[i]->getOutputTex() });
	}

	std::vector<TextureInfo> marioTexInfo = shadowMap;
	marioTexInfo.push_back({ "u_texture",marioTex });
	addPass(&m_camera, mario, CSM, marioTexInfo);

	std::vector<TextureInfo> planeTexInfo = shadowMap;
	planeTexInfo.push_back({ "u_texture", planeTex });
	addPass(&m_camera, plane, CSM, planeTexInfo);

	////add Pass4
	//std::vector<TextureInfo> texInfo4;
	//texInfo4.push_back({ "u_texture", planeTex });
	//auto shadowedPass2 = addShadowedPass(&m_camera, plane, PCSS, texInfo4);
	//shadowedPass2->addShadowMap({ "u_depth",framebuffer->getOutputTex() });


	//setup post shading pass


}
