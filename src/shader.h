#ifndef SHADER_H
#define SHADER_H

#include <imgui_impl_opengl3_loader.h>
#include <string>
#include <unordered_map>

extern "C" {
extern void glUniform1f(GLint location, GLfloat v0);
extern void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
extern void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
}

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
    void setVec2(const char* name, float x, float y);
    void setMat4(const char* name, const float* value);
    void setMat3(const char* name, const float* value);

private:
    GLuint program_ = 0;
    std::unordered_map<std::string, GLint> uniformCache_;

    GLint getUniformLocation(const char* name);
    void clear();
};

#endif // SHADER_H
