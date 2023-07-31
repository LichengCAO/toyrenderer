#include "Drawable.h"
Mesh::Mesh(const char* objFile) {
	std::cout << "load obj from: " << objFile << std::endl;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string res = tinyobj::LoadObj(shapes, materials, objFile);

	if (res.size() == 0/*no error*/ && shapes.size() > 0) {
		std::vector<float>& positions = shapes[0].mesh.positions;
		std::vector<float>& normals = shapes[0].mesh.normals;
		std::vector<float>& uvs = shapes[0].mesh.texcoords;
		std::vector<unsigned int>& indices = shapes[0].mesh.indices;
		for (unsigned int ui : indices)
		{
			glIndices.push_back(ui);
		}
		bool normalsExist = normals.size() > 0;
		for (int x = 0; x < positions.size(); x += 3)
		{
			glPos.push_back(glm::vec4(positions[x], positions[x + 1], positions[x + 2], 1.f));
			if (normalsExist)
			{
				glNor.push_back(glm::vec3(normals[x], normals[x + 1], normals[x + 2]));
			}
		}
		bool uvsExist = uvs.size() > 0;
		if (uvsExist)
		{
			for (int x = 0; x < uvs.size(); x += 2)
			{
				glUV.push_back(glm::vec2(uvs[x], uvs[x + 1]));
			}
		}
	}
	else {
		std::cout << res << std::endl;
	}
	create();
}
void Mesh::create() {
	destroy();
	m_idxBufSize = glIndices.size();
	glGenBuffers(1, &m_idxBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, glIndices.size() * sizeof(unsigned int), glIndices.data(), GL_STATIC_DRAW);
	auto& bufVert = addAttrib("vs_pos");
	glBindBuffer(GL_ARRAY_BUFFER, bufVert);
	glBufferData(GL_ARRAY_BUFFER, glPos.size() * sizeof(glm::vec4), glPos.data(), GL_STATIC_DRAW);

	bool normalsExist = glNor.size() > 0;
	if (normalsExist) {
		auto& buf = addAttrib("vs_norm");
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		glBufferData(GL_ARRAY_BUFFER, glNor.size() * sizeof(glm::vec3), glNor.data(), GL_STATIC_DRAW);
	}
	bool uvsExist = glUV.size() > 0;
	if (uvsExist) {
		auto& buf = addAttrib("vs_uv");
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		glBufferData(GL_ARRAY_BUFFER, glUV.size() * sizeof(glm::vec2), glUV.data(), GL_STATIC_DRAW);
	}
	std::cout << "Mesh created" << std::endl;
	std::cout << std::endl;
}
void Mesh::draw(ShaderProgram* shader) {
	shader->setUnifMat4("u_model", getModel());
	shader->setUnifMat3("u_modelInvTr", getModelInvTr());
	int posAttr = shader->findAttrib("vs_pos");
	int normAttr = shader->findAttrib("vs_norm");
	int uvAttr = shader->findAttrib("vs_uv");
	debugLog();
	if (posAttr != -1 && useAttrib("vs_pos")) {
		glEnableVertexAttribArray(posAttr);
		glVertexAttribPointer(posAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	debugLog();
	if (normAttr != -1 && useAttrib("vs_norm")) {
		glEnableVertexAttribArray(normAttr);
		glVertexAttribPointer(normAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	debugLog();
	if (uvAttr != -1 && useAttrib("vs_uv")) {
		glEnableVertexAttribArray(uvAttr);
		glVertexAttribPointer(uvAttr, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	debugLog();
	useIdx();
	glDrawElements(drawMode(), idBufferSize(), GL_UNSIGNED_INT, 0);
	debugLog();
	if (posAttr != -1)glDisableVertexAttribArray(posAttr);
	if (normAttr != -1)glDisableVertexAttribArray(normAttr);
	if (uvAttr != -1)glDisableVertexAttribArray(uvAttr);
}