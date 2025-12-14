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

void UnifyVertexAttribute(cyTriMesh& mesh, std::vector<VertexModel>& verticies, std::vector<unsigned int>& indices){
    int facecount = mesh.NF();
    verticies.clear();
    verticies.resize(mesh.NV());
    indices.clear();
    indices.resize(mesh.NF() * 3);

    if (facecount > 0){
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
    else{
        for(int i = 0; i<mesh.NV(); i++){
            verticies[i].pos = mesh.V(i);
            verticies[i].normal = mesh.VN(i);
            verticies[i].st = mesh.VT(i);
        }
    }
}

struct VertexArray{
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    std::vector<VertexModel> vertices;
    std::vector<unsigned int> indicies;

    VertexArray(){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    ~VertexArray(){
        if (VAO != 0){
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0){
            glDeleteBuffers(1, &VBO);
        }
        if (EBO != 0){
            glDeleteBuffers(1, &EBO);
        }
    }

    void Create(cyTriMesh& mesh, GLuint program){
        Bind();

        UnifyVertexAttribute(mesh, vertices, indicies);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexModel), vertices.data(), GL_STATIC_DRAW);

        GLuint pos = glGetAttribLocation(program, "pos");
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)0);
        glEnableVertexAttribArray(pos);

        GLuint normal = glGetAttribLocation(program, "normal");
        glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)(offsetof(VertexModel, normal)));
        glEnableVertexAttribArray(normal);

        GLuint texcoord = glGetAttribLocation(program, "texcoord");
        glVertexAttribPointer(texcoord, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)(offsetof(VertexModel, st)));
        glEnableVertexAttribArray(texcoord);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (indicies.size() > 0){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned int), indicies.data(), GL_STATIC_DRAW);    
        }

        Unbind();
    }

    void Bind(){
        glBindVertexArray(VAO);
    }

    void Unbind(){
        glBindVertexArray(0);
    }

    void Draw(){
        Bind();
        if (EBO > 0){
            glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
        }
        else{
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }
        Unbind();
    }
    
};