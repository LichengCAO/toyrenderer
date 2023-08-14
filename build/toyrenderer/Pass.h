#pragma once
#include "Camera.h"
#include "Texture.h" 
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Drawable.h"
#include <vector>

//setup textures
struct TextureInfo {
	std::string name;
	Texture* tex;
};

class Pass
{
protected:
	const Camera* m_camera;
	Drawable* m_drawable;
	ShaderProgram* m_shader;
	std::vector<TextureInfo> m_texInfos;
	FrameBuffer* m_frameBuf;
	bool m_forceClear;
	bool needClearBuffer()const;
public:
	static FrameBuffer* u_bufferInUse;//shared by all child class https://blog.csdn.net/weixin_43356308/article/details/116371338
	Pass();
	Pass(const Camera*, Drawable*, ShaderProgram*, const std::vector<TextureInfo>&, 
		FrameBuffer* frameBuf = nullptr, bool forceClear = false);
	virtual void run();
};

class ShadowedPass
	:public Pass
{
protected:
	float calBiasA(const Texture* shadowTex);//https://zhuanlan.zhihu.com/p/370951892
	std::vector<std::pair<TextureInfo,float>> m_shadowInfos;
public:
	ShadowedPass(const Camera*, Drawable*, ShaderProgram*, const std::vector<TextureInfo>&, FrameBuffer* frameBuf = nullptr, bool forceClear = false);
	void addShadowMap(const TextureInfo& shadowMap);
	virtual void run()override;
};

class HizPass
	:public Pass
{
private:
	GBuffer* m_GBuffer;
public:
	HizPass(Screen*, ShaderProgram*, GBuffer* frameBuf);
	virtual void run()override;
};

class TAAPass
	:public Pass
{
private:
	bool m_useFirstRecord;
	std::vector<std::unique_ptr<FrameBuffer>> m_recordBuf;
	glm::mat4 m_prevViewProj;
	ShaderProgram* m_simplePassShader;
public:
	TAAPass(const Camera*, Screen*, ShaderProgram*, ShaderProgram* simplePass,const std::vector<TextureInfo>&, FrameBuffer* frameBuf = nullptr);
	virtual void run()override;
};

