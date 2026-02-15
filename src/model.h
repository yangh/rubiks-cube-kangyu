#ifndef MODEL_H
#define MODEL_H

#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    void Draw();

private:
    void setupMesh();
};

class Model {
public:
    std::vector<Mesh> meshes;

    Model(const std::string& path);

    void Draw();

private:
    void loadModel(const std::string& path);
};

#endif // MODEL_H
