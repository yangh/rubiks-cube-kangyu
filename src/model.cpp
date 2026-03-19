#include "model.h"
#include <GL/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vertices(vertices), indices(indices) {
    setupMesh();
}

void Mesh::setupMesh() {
    // For fixed-function OpenGL, we'll store the data in vectors
    // and draw using glBegin/glEnd (not optimal but simple)
}

void Mesh::Draw() {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); i++) {
        unsigned int idx = indices[i];
        if (idx < vertices.size()) {
            // Set normal
            glNormal3f(vertices[idx].Normal.x, vertices[idx].Normal.y, vertices[idx].Normal.z);
            // Set vertex
            glVertex3f(vertices[idx].Position.x, vertices[idx].Position.y, vertices[idx].Position.z);
        }
    }
    glEnd();
}

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::Draw() {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw();
    }
}

void Model::loadModel(const std::string& path) {
    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec3> tempNormals;

    std::vector<Vertex> meshVertices;
    std::vector<unsigned int> meshIndices;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR::MODEL::FILE_NOT_SUCCESSFULLY_READ: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            tempVertices.push_back(position);
        } else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        } else if (prefix == "f") {
            std::string vertexData;
            std::vector<int> vertexIndices;
            std::vector<int> normalIndices;

            while (iss >> vertexData) {
                // Parse face data (vertex//normal format)
                size_t firstSlash = vertexData.find('/');
                size_t secondSlash = vertexData.find('/', firstSlash + 1);

                if (firstSlash != std::string::npos && secondSlash != std::string::npos) {
                    // vertex//normal format
                    int vIndex = std::stoi(vertexData.substr(0, firstSlash));
                    int nIndex = std::stoi(vertexData.substr(secondSlash + 1));

                    vertexIndices.push_back(vIndex - 1); // OBJ uses 1-based indexing
                    normalIndices.push_back(nIndex - 1);
                } else if (firstSlash != std::string::npos) {
                    // vertex/texture format (or vertex only)
                    int vIndex = std::stoi(vertexData.substr(0, firstSlash));
                    vertexIndices.push_back(vIndex - 1);
                } else {
                    // vertex only
                    int vIndex = std::stoi(vertexData);
                    vertexIndices.push_back(vIndex - 1);
                }
            }

            // Create triangles from face (quad faces need to be triangulated)
            for (size_t i = 1; i < vertexIndices.size() - 1; i++) {
                // First triangle
                int vi0 = vertexIndices[0];
                int vi1 = vertexIndices[i];
                int vi2 = vertexIndices[i + 1];

                Vertex v;
                v.Position = tempVertices[vi0];
                v.Normal = normalIndices.empty() ? glm::vec3(0.0f) : tempNormals[normalIndices[0]];
                meshVertices.push_back(v);
                meshIndices.push_back(meshIndices.size());

                v.Position = tempVertices[vi1];
                v.Normal = normalIndices.empty() ? glm::vec3(0.0f) : tempNormals[normalIndices[i]];
                meshVertices.push_back(v);
                meshIndices.push_back(meshIndices.size());

                v.Position = tempVertices[vi2];
                v.Normal = normalIndices.empty() ? glm::vec3(0.0f) : tempNormals[normalIndices[i + 1]];
                meshVertices.push_back(v);
                meshIndices.push_back(meshIndices.size());
            }
        }
    }

    file.close();

    if (!meshVertices.empty()) {
        meshes.push_back(Mesh(meshVertices, meshIndices));
    }
}
