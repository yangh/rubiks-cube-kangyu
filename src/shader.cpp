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
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const char* name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec3(const char* name, float x, float y, float z) {
    glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setVec2(const char* name, float x, float y) {
    glUniform2f(getUniformLocation(name), x, y);
}

void Shader::setMat4(const char* name, const float* value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
}

void Shader::setMat3(const char* name, const float* value) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, value);
}
