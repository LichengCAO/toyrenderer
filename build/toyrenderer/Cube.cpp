#include "Drawable.h"
Cube::Cube()
{
	create();
}
void Cube::create() {
	destroy();
    std::vector<unsigned int> indices;
    for (int i = 0;i < 6;++i) {
        indices.push_back(i * 4);
        indices.push_back(i * 4 + 1);
        indices.push_back(i * 4 + 2);

        indices.push_back(i * 4);
        indices.push_back(i * 4 + 2);
        indices.push_back(i * 4 + 3);
    }
	glGenBuffers(1, &m_idxBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
	m_idxBufSize = 36;
	float vertices[] = {
		//Front face
		//UR
		1.0f, 1.0f, 1.0f, 1.0f,
		//LR
		1.0f, 0.0f, 1.0f, 1.0f,
		//LL
		0.0f, 0.0f, 1.0f, 1.0f,
		//UL
		0.0f, 1.0f, 1.0f, 1.0f,

		//Right face
		//UR
		1.0f, 1.0f, 0.0f, 1.0f,
		//LR
		1.0f, 0.0f, 0.0f, 1.0f,
		//LL
		1.0f, 0.0f, 1.0f, 1.0f,
		//UL
		1.0f, 1.0f, 1.0f, 1.0f,

		//Left face
		//UR
		0.0f, 1.0f, 1.0f, 1.0f,
		//LR
		0.0f, 0.0f, 1.0f, 1.0f,
		//LL
		0.0f, 0.0f, 0.0f, 1.0f,
		//UL
		0.0f, 1.0f, 0.0f, 1.0f,

		//Back face
		//UR
		0.0f, 1.0f, 0.0f, 1.0f,
		//LR
		0.0f, 0.0f, 0.0f, 1.0f,
		//LL
		1.0f, 0.0f, 0.0f, 1.0f,
		//UL
		1.0f, 1.0f, 0.0f, 1.0f,

		//Top face
		//UR
		1.0f, 1.0f, 0.0f, 1.0f,
		//LR
		1.0f, 1.0f, 1.0f, 1.0f,
		//LL
		0.0f, 1.0f, 1.0f, 1.0f,
		//UL
		0.0f, 1.0f, 0.0f, 1.0f,

		//Bottom face
		//UR
		1.0f, 0.0f, 1.0f, 1.0f,
		//LR
		1.0f, 0.0f, 0.0f, 1.0f,
		//LL
		0.0f, 0.0f, 0.0f, 1.0f,
		//UL
		0.0f, 0.0f, 1.0f, 1.0f
	};
	auto& bufVert = addAttrib("vs_pos");
	glBindBuffer(GL_ARRAY_BUFFER, bufVert);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	float norms[] = {
		//front
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		//Right
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		//Left
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		//Back
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		//Top
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		//Bottom
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		0, -1, 0
	};
	auto& bufNorm = addAttrib("vs_norm");
	glBindBuffer(GL_ARRAY_BUFFER, bufNorm);
	glBufferData(GL_ARRAY_BUFFER, sizeof(norms), norms, GL_STATIC_DRAW);
	float uvs[] = {
		//front
		1.f,1.f,
		1.f,0.f,
		0.f,0.f,
		0.f,1.f,
		//Right
		1.f,1.f,
		1.f,0.f,
		0.f,0.f,
		0.f,1.f,
		//Left
		1.f,1.f,
		1.f,0.f,
		0.f,0.f,
		0.f,1.f,
		//Back
		1.f,1.f,
		1.f,0.f,
		0.f,0.f,
		0.f,1.f,
		//Top
		1.f,1.f,
		1.f,0.f,
		0.f,0.f,
		0.f,1.f,
		//Bottom
		1.f,1.f,
		1.f,0.f,
		0.f,0.f,
		0.f,1.f,
	};
	auto& bufUV = addAttrib("vs_uv");
	glBindBuffer(GL_ARRAY_BUFFER, bufUV);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	std::cout << "Cube created" << std::endl;
	std::cout << std::endl;
}

void Cube::draw(ShaderProgram* shader) {
	if (debug)fillAttrCheck(shader);
	shader->setUnifMat4("u_model", getModel());
	shader->setUnifMat3("u_modelInvTr", getModelInvTr());
	int posAttr = shader->findAttrib("vs_pos");
	int normAttr = shader->findAttrib("vs_norm");
	int uvAttr = shader->findAttrib("vs_uv");
	if (posAttr != -1 && useAttrib("vs_pos")) {
		glEnableVertexAttribArray(posAttr);
		glVertexAttribPointer(posAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	if (normAttr != -1 && useAttrib("vs_norm")) {
		glEnableVertexAttribArray(normAttr);
		glVertexAttribPointer(normAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
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