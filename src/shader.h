#ifndef SHADER_H
#define SHADER_H

#include "gl_loader.h"
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader();
    ~Shader();

    bool compileShaderFromString(GLenum type, const char* source);
    bool linkProgram();
    void use();
    void unuse();

    GLuint getProgram() const { return program_; }
    bool isValid() const { return program_ != 0; }

    void setInt(const char* name, int value);
    void setFloat(const char* name, float value);
    void setVec3(const char* name, float x, float y, float z);
    void setMat4(const char* name, const float* value);
    void setMat3(const char* name, const float* value);

private:
    GLuint program_ = 0;
    std::unordered_map<std::string, GLint> uniformCache_;

    GLint getUniformLocation(const char* name);
    void clear();
};

#endif // SHADER_H
