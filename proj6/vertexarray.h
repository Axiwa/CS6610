#pragma once

#include "cy/cyVector.h"
#include "cy/cyTriMesh.h"
#include <vector>
#include <glad/glad.h>

struct VertexModel {
	cyVec3f pos;
	cyVec3f normal;
	cyVec3f st;
};

void UnifyVertexAttribute(cyTriMesh& mesh, std::vector<VertexModel>& verticies, std::vector<int>& indices){
    int facecount = mesh.NF();
    verticies.clear();
    verticies.resize(mesh.NV());
    indices.resize(mesh.NF() * 3);

    for(int i = 0; i<facecount; i++){
        cyTriMesh::TriFace vertexindex = mesh.F(i);
        cyTriMesh::TriFace normalindex = mesh.FN(i);
        cyTriMesh::TriFace uvindex = mesh.FT(i);

        VertexModel vertex;

        indices[3*i] = vertexindex.v[0];
        indices[3*i+1] = vertexindex.v[1];
        indices[3*i+2] = vertexindex.v[2];

        vertex.pos = mesh.V(vertexindex.v[0]);
        vertex.normal = mesh.VN(normalindex.v[0]);
        vertex.st = mesh.VT(uvindex.v[0]);
        verticies[vertexindex.v[0]] = vertex;

        vertex.pos = mesh.V(vertexindex.v[1]);
        vertex.normal = mesh.VN(normalindex.v[1]);
        vertex.st = mesh.VT(uvindex.v[1]);
        verticies[vertexindex.v[1]] = vertex;

        vertex.pos = mesh.V(vertexindex.v[2]);
        vertex.normal = mesh.VN(normalindex.v[2]);
        vertex.st = mesh.VT(uvindex.v[2]);
        verticies[vertexindex.v[2]] = vertex;
    }
}

struct VertexArray{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    std::vector<VertexModel> vertices;
    std::vector<int> indicies;

    VertexArray(){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    ~VertexArray(){
        if (VAO != 0){
            glDeleteVertexArrays(1, &VAO);
        }
    }

    void Create(cyTriMesh& mesh){
        Bind();

        UnifyVertexAttribute(mesh, vertices, indicies);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.)

        Unbind();
    }

    void SetAttribute(){

    }

    void Bind(){
        glBindVertexArray(VAO);
    }

    void Unbind(){
        glBindVertexArray(0);
    }
};