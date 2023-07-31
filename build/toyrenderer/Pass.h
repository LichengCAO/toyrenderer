#pragma once
#include "Camera.h"
#include "Texture.h" 
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Drawable.h"
#include <vector>

//setup textures
//set
struct TextureInfo {
	std::string name;
	Texture* tex;
};
class Pass
{
private:
	const Camera* m_camera;
	Drawable* m_drawable;
	ShaderProgram* m_shader;
	std::vector<TextureInfo> m_texInfos;
public:
	Pass(const Camera*, Drawable*, ShaderProgram*, const std::vector<TextureInfo>&);
	void run();
};

