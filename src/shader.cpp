#include "shader.h"
#include <iostream>
#include <cstring>

Shader::Shader() : program_(0) {
}

Shader::~Shader() {
    clear();
}

void Shader::clear() {
    if (program_ != 0) {
        GL_LOADER.glDeleteProgram(program_);
        program_ = 0;
    }
    uniformCache_.clear();
}

bool Shader::compileShaderFromString(GLenum type, const char* source) {
    if (program_ == 0) {
        program_ = GL_LOADER.glCreateProgram();
    }

    GLuint shader = GL_LOADER.glCreateShader(type);
    GL_LOADER.glShaderSource(shader, 1, &source, nullptr);
    GL_LOADER.glCompileShader(shader);

    GLint success;
    GL_LOADER.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        GL_LOADER.glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        GL_LOADER.glDeleteShader(shader);
        return false;
    }

    GL_LOADER.glAttachShader(program_, shader);
    GL_LOADER.glDeleteShader(shader);
    return true;
}

bool Shader::linkProgram() {
    if (program_ == 0) {
        std::cerr << "No shaders attached to program" << std::endl;
        return false;
    }

    GL_LOADER.glLinkProgram(program_);

    GLint success;
    GL_LOADER.glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        GL_LOADER.glGetProgramInfoLog(program_, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }

    return true;
}

void Shader::use() {
    if (program_ != 0) {
        GL_LOADER.glUseProgram(program_);
    }
}

void Shader::unuse() {
    GL_LOADER.glUseProgram(0);
}

GLint Shader::getUniformLocation(const char* name) {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) {
        return it->second;
    }

    GLint loc = GL_LOADER.glGetUniformLocation(program_, name);
    uniformCache_[name] = loc;
    return loc;
}

void Shader::setInt(const char* name, int value) {
    GL_LOADER.glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const char* name, float value) {
    GL_LOADER.glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec3(const char* name, float x, float y, float z) {
    GL_LOADER.glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setMat4(const char* name, const float* value) {
    GL_LOADER.glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
}

void Shader::setMat3(const char* name, const float* value) {
    GL_LOADER.glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, value);
}
