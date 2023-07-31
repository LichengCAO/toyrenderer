#include "Drawable.h"
Plane::Plane() {
	create();
}
void Plane::create() {
	destroy();
	unsigned int indices[] = {
		0, 1, 2,
		3, 4, 5
	};
	glGenBuffers(1, &m_idxBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	m_idxBufSize = 6;
	float vertices[] = {
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f,
	};
	auto& bufVert = addAttrib("vs_pos");
	glBindBuffer(GL_ARRAY_BUFFER, bufVert);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	float norms[] = {
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	};
	auto& bufNorm = addAttrib("vs_norm");
	glBindBuffer(GL_ARRAY_BUFFER, bufNorm);
	glBufferData(GL_ARRAY_BUFFER, sizeof(norms), norms, GL_STATIC_DRAW);
	float uvs[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	auto& bufUV = addAttrib("vs_uv");
	glBindBuffer(GL_ARRAY_BUFFER, bufUV);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	std::cout << "Plane created" << std::endl;
	std::cout << std::endl;
}
void Plane::draw(ShaderProgram* shader) {
	int posAttr = shader->findAttrib("vs_pos");
	int normAttr = shader->findAttrib("vs_norm");
	int uvAttr = shader->findAttrib("vs_uv");
	if (posAttr != -1 && useAttrib("vs_pos")) {
		glEnableVertexAttribArray(posAttr);
		glVertexAttribPointer(posAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	if (normAttr != -1 && useAttrib("vs_norm")) {
		glEnableVertexAttribArray(normAttr);
		glVertexAttribPointer(normAttr,3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	if (uvAttr != -1 && useAttrib("vs_uv")) {
		glEnableVertexAttribArray(uvAttr);
		glVertexAttribPointer(uvAttr, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	useIdx();
	glDrawElements(drawMode(), idBufferSize(), GL_UNSIGNED_INT, 0);
	debugLog();
	if (posAttr != -1)glDisableVertexAttribArray(posAttr);
	if (normAttr != -1)glDisableVertexAttribArray(normAttr);
	if (uvAttr != -1)glDisableVertexAttribArray(uvAttr);
}