#include "shader.h"
#include <iostream>

Shader::Shader() : program_(0) {
}

Shader::~Shader() {
    clear();
}

void Shader::clear() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
    uniformCache_.clear();
}

bool Shader::compileShaderFromString(GLenum type, const char* source) {
    if (program_ == 0) {
        program_ = glCreateProgram();
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return false;
    }

    glAttachShader(program_, shader);
    glDeleteShader(shader);
    return true;
}

bool Shader::linkProgram() {
    if (program_ == 0) {
        std::cerr << "No shaders attached to program" << std::endl;
        return false;
    }

    glLinkProgram(program_);

    GLint success;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program_, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }

    return true;
}

void Shader::use() {
    if (program_ != 0) {
        glUseProgram(program_);
    }
}

void Shader::unuse() {
    glUseProgram(0);
}

GLint Shader::getUniformLocation(const char* name) {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) {
        return it->second;
    }

    GLint loc = glGetUniformLocation(program_, name);
    uniformCache_[name] = loc;
    return loc;
}

void Shader::setInt(const char* name, int value) {
    GLint loc = getUniformLocation(name);
    if (loc >= 0) glUniform1i(loc, value);
}

void Shader::setFloat(const char* name, float value) {
    GLint loc = getUniformLocation(name);
    if (loc >= 0) glUniform1f(loc, value);
}

void Shader::setVec3(const char* name, float x, float y, float z) {
    GLint loc = getUniformLocation(name);
    if (loc >= 0) glUniform3f(loc, x, y, z);
}

void Shader::setVec2(const char* name, float x, float y) {
    GLint loc = getUniformLocation(name);
    if (loc >= 0) glUniform2f(loc, x, y);
}

void Shader::setMat4(const char* name, const float* value) {
    GLint loc = getUniformLocation(name);
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, value);
}

void Shader::setMat3(const char* name, const float* value) {
    GLint loc = getUniformLocation(name);
    if (loc >= 0) glUniformMatrix3fv(loc, 1, GL_FALSE, value);
}

GLint Shader::getLocation(const char* name) {
    return getUniformLocation(name);
}

void Shader::setFloatAt(GLint loc, float value) {
    if (loc >= 0) glUniform1f(loc, value);
}

void Shader::setVec3At(GLint loc, float x, float y, float z) {
    if (loc >= 0) glUniform3f(loc, x, y, z);
}
