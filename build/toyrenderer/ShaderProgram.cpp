#include "ShaderProgram.h"
ShaderProgram::~ShaderProgram() {
    glDeleteProgram(prog);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}
void ShaderProgram::printShaderInfoLog(int shader)const {
    char infoLog[512];
    glGetProgramInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
}
void ShaderProgram::printLinkInfoLog(int prog)const {
    char infoLog[512];
    glGetProgramInfoLog(prog, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
}
void ShaderProgram::printUnifErrorLog(const std::string& name)const {
    if(errorLog)std::cout << "Error: could not find shader variable with name " << name << std::endl;
}
ShaderProgram::ShaderProgram(const char* vertFile, const char* fragFile){
    std::cout << "create vertex shader from: " << vertFile << std::endl;
    std::cout << "create fragment shader from: " << fragFile << std::endl;
    // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    prog = glCreateProgram();
    // Get the body of text stored in our two .glsl files
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vShaderFile.open(vertFile);
        fShaderFile.open(fragFile);
        std::stringstream vShaderStream, fShaderStream;
        // 读取文件的缓冲内容到数据流中
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // 关闭文件处理器
        vShaderFile.close();
        fShaderFile.close();
        // 转换数据流到string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        if(vShaderFile.is_open())vShaderFile.close();
        if (fShaderFile.is_open())fShaderFile.close();
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }


    // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
    const char* vertSource = vertexCode.c_str();
    const char* fragSource = fragmentCode.c_str();
    
    glShaderSource(vertShader, 1, &vertSource, 0);
    glShaderSource(fragShader, 1, &fragSource, 0);
    // Tell OpenGL to compile the shader text stored above
    glCompileShader(vertShader);
    glCompileShader(fragShader);
    // Check if everything compiled OK
    int compiled;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "vertex shader" << ' ';
        printShaderInfoLog(vertShader);
    }
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "fragment shader" << ' ';
        printShaderInfoLog(fragShader);
    }

    // Tell prog that it manages these particular vertex and fragment shaders
    glAttachShader(prog, vertShader);
    glAttachShader(prog, fragShader);
    glLinkProgram(prog);

    // Check for linking success
    int linked;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        printLinkInfoLog(prog);
    }
}
void ShaderProgram::addAttrib(const char* name) {
    std::cout << "try to find " << name << ":";
    m_attribs[name] = glGetAttribLocation(prog, name);
    if (m_attribs[name] == -1) {
        std::cout << " failed" << std::endl;
    }
    else {
        std::cout << " success" << std::endl;
    }
}
int ShaderProgram::findAttrib(const std::string& name)const {
    useMe();
    int ans = m_attribs.find(name) != m_attribs.end() ? m_attribs.at(name) : -1;
    return ans;
}
void ShaderProgram::addUniform(const char* name) {
    std::cout << "try to find " << name << ":";
    m_unifs[name] = glGetUniformLocation(prog, name);
    if (m_unifs[name] == -1) {
        std::cout << " failed" << std::endl;
    }
    else {
        std::cout << " success" << std::endl;
    }
}
void ShaderProgram::setUnifMat4(const std::string& name, const glm::mat4& m)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniformMatrix4fv(handle, 1, GL_FALSE, &m[0][0]);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
void ShaderProgram::setUnifMat3(const std::string& name, const glm::mat3& m)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniformMatrix3fv(handle, 1, GL_FALSE, &m[0][0]);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
void ShaderProgram::setUnifVec2(const std::string& name, const glm::vec2& v)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniform2fv(handle, 1, &v[0]);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
void ShaderProgram::setUnifVec3(const std::string& name, const glm::vec3& v)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniform3fv(handle, 1, &v[0]);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
void ShaderProgram::setUnifFloat(const std::string& name, float f)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniform1f(handle, f);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
void ShaderProgram::setUnifInt(const std::string& name, int i)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniform1i(handle, i);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
void ShaderProgram::setUnifBool(const std::string& name, bool i)const {
    useMe();
    try {
        int handle = m_unifs.at(name);
        if (handle != -1) {
            glUniform1i(handle, (bool)i);
        }
    }
    catch (std::out_of_range& e) {
        printUnifErrorLog(name);
    }
}
//void ShaderProgram::draw(Drawable& d) {
//    useMe();
//    int handle = -1;
//    if (m_attribs.find("vs_pos")!=m_attribs.end()&&(handle = m_attribs["vs_pos"]) != -1 && d.useAttrib("vs_pos")) {
//        glEnableVertexAttribArray(handle);
//        glVertexAttribPointer(handle, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
//    }
//    d.useIdx();
//    glDrawElements(d.drawMode(), d.idBufferSize(), GL_UNSIGNED_INT, 0);
//    debugLog();
//    if (handle != -1)glDisableVertexAttribArray(handle);
//}
void ShaderProgram::debugLog()const {
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