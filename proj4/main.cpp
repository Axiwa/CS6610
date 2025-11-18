#include <iostream>
#include <glad/glad.h> 
#include "cy/cyTriMesh.h"
#include "cy/cyMatrix.h"
#include <GLFW/glfw3.h>
#include "lodepng.h"
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

#ifndef M_PI
#define M_PI 3.1415926
#endif

#define MAX_PATH 128

double lastX, lastY;
bool isDraggingLeft = false;
bool isDraggingRight = false;

struct Vertex {
    cy::Vec3f pos;
    cy::Vec3f normal;
    cy::Vec3f st;
};

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDraggingLeft = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE) {
            isDraggingLeft = false;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isDraggingRight = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE) {
            isDraggingRight = false;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */

    return buf; /* Return the buffer */
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

bool LoadShader(const char* vertfile, const char* fragfile, GLuint& vertshader, GLuint& fragshader){
    if (vertshader != 0){
        glDeleteShader(vertshader);
    }
    if (fragshader != 0){
        glDeleteShader(fragshader);
    }

    int success;
    int maxLength;

    GLchar* vertsource;
    GLchar* fragsource;

    vertsource = filetobuf(vertfile);
    fragsource = filetobuf(fragfile);

    vertshader = glCreateShader(GL_VERTEX_SHADER);
    fragshader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertshader, 1, (const GLchar**)&vertsource, 0);
    glShaderSource(fragshader, 1, (const GLchar**)&fragsource, 0);

    glCompileShader(vertshader);
    glGetShaderiv(vertshader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderiv(vertshader, GL_INFO_LOG_LENGTH, &maxLength);
        char* vertexInfoLog;

        /* The maxLength includes the NULL character */
        vertexInfoLog = (char*)malloc(maxLength);
        glGetShaderInfoLog(vertshader, maxLength, &maxLength, vertexInfoLog);
        std::cout << vertexInfoLog << std::endl;
        free(vertexInfoLog);
        return false;
    }

    glCompileShader(fragshader);
    glGetShaderiv(fragshader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderiv(fragshader, GL_INFO_LOG_LENGTH, &maxLength);
        char* fragInfoLog;

        fragInfoLog = (char*)malloc(maxLength);
        glGetShaderInfoLog(fragshader, maxLength, &maxLength, fragInfoLog);
        std::cout << fragInfoLog << std::endl;
        free(fragInfoLog);
        return false;
    }

    return true;
}

int main(int argc, char** argv){
    if (argc < 2){
        std::cerr<<"Input format is proj [model file name] ([texture file name])"<<std::endl;
        exit(1);
    }
    std::vector<unsigned char> diffusetexbuffer;
    std::vector<unsigned char> speculartexbuffer;
    std::vector<unsigned char> diffusedata;
    std::vector<unsigned char> speculardata;
    unsigned int width_d, height_d, width_s, height_s;

    std::string modelfile = argv[1];
    bool loadMtl = false;

    if (argc > 2){
        loadMtl = true;
    }

    cy::TriMesh mesh;
    if (!mesh.LoadFromFileObj(modelfile.c_str(), loadMtl)){
        std::cerr<<"Load modelfile failed!\n";
        exit(1);
    }
    else{
        std::cout<<"Successfully loaded "<<modelfile<<std::endl;
    }

    // Deal with different index buffer
    std::vector<Vertex> vertexbuffer(mesh.NV());
    for(int i = 0; i < mesh.NF(); i++){
        Vertex vet;
        
        cy::TriMesh::TriFace vertindex = mesh.F(i);
        cy::TriMesh::TriFace normalindex = mesh.FN(i);
        cy::TriMesh::TriFace tecindex = mesh.FT(i);

        vet.pos = mesh.V(vertindex.v[0]);
        vet.normal = mesh.VN(normalindex.v[0]);
        vet.st = mesh.VT(tecindex.v[0]);
        vertexbuffer[vertindex.v[0]] = vet;

        vet.pos = mesh.V(vertindex.v[1]);
        vet.normal = mesh.VN(normalindex.v[1]);
        vet.st = mesh.VT(tecindex.v[1]);
        vertexbuffer[vertindex.v[1]] = vet;

        vet.pos = mesh.V(vertindex.v[2]);
        vet.normal = mesh.VN(normalindex.v[2]);
        vet.st = mesh.VT(tecindex.v[2]);
        vertexbuffer[vertindex.v[2]] = vet;
    }

    // Material
    if (mesh.NM() > 0) {
        // Only 1 mat for now
        std::string map_kd = mesh.M(0).map_Kd;
        std::string map_ks = mesh.M(0).map_Ks;
        std::string parent = "D:/learn/CS6610/proj4/teapot/";

        lodepng::load_file(diffusetexbuffer, parent + map_kd);
        lodepng::decode(diffusedata, width_d, height_d, diffusetexbuffer);
        lodepng::load_file(speculartexbuffer, parent + map_ks);
        lodepng::decode(speculardata, width_s, height_s, speculartexbuffer);
    }

    // Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 720;
    int height = 720;
    GLFWwindow* window = glfwCreateWindow(width, height, "My proj 4", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // LOAD GL PROGRAM
    GLuint program = glCreateProgram();

    // Bind Shader
    GLuint vertshader, fragshader;
    if (!LoadShader("D:/learn/CS6610/proj4/shader.vert", "D:/learn/CS6610/proj4/shader.frag", vertshader, fragshader)){
        std::cerr<<"Load shader failed!"<<std::endl;
        exit(1);
    }

    glAttachShader(program, vertshader);
    glAttachShader(program, fragshader);
    glLinkProgram(program);

    int success;
    int maxLength;
    glGetProgramiv(program, GL_LINK_STATUS, (int*)&success);
    if (success == GL_FALSE)
    {
        /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        /* The maxLength includes the NULL character */
        char* shaderProgramInfoLog = (char*)malloc(maxLength);

        /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
        glGetProgramInfoLog(program, maxLength, &maxLength, shaderProgramInfoLog);

        /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
        std::cout << shaderProgramInfoLog << std::endl;
        free(shaderProgramInfoLog);
        exit(1);
    }

    // VAO VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexbuffer.size(), vertexbuffer.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, st)));
            glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.NF() * 3, &mesh.F(0), GL_STATIC_DRAW);

    glBindVertexArray(0);

    GLuint diffuseID = 0;
    GLuint specularID = 0;

    if (diffusedata.size() > 0 && speculardata.size() > 0) {
        glGenTextures(1, &diffuseID);
        glBindTexture(GL_TEXTURE_2D, diffuseID);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width_d,
            height_d,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            diffusedata.data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &specularID);
        glBindTexture(GL_TEXTURE_2D, specularID);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width_s,
            height_s,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            speculardata.data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // MVP matrix
    cy::Matrix4f model =cy::Matrix4f::Identity();
    model.Zero();
    model.cell[2] = 1;
    model.cell[4] = 1;
    model.cell[9] = 1;
    model.cell[15] = 1;
    cy::Vec3f boundBoxMin = mesh.GetBoundMin();
    cy::Vec3f boundBoxMax = mesh.GetBoundMax();
    cy::Vec3f offset = (boundBoxMax + boundBoxMin) / 2;
    model.AddTranslation(-offset);

    cy::Matrix4f view = cy::Matrix4f::Identity();
    cy::Matrix4f proj = cy::Matrix4f::Identity();
    
    cy::Matrix4f rotation1 = cy::Matrix4f::Identity();
    cy::Matrix4f rotation2 = cy::Matrix4f::Identity();

    float phi = 0;
    float theta = 0;
    float distance = 50;

    float light_phi = 0;
    float light_theta = 0;

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        if (isDraggingRight){
            double nowX, nowY;
            glfwGetCursorPos(window, &nowX, &nowY);
            
            double dist = sqrt((lastX - nowX) * (lastX - nowX) + (lastY - nowY) * (lastY - nowY)) / 50;
            if (lastX - nowX > 0){
                distance += dist;
            }
            else{
                distance -= dist;
            }
            distance = std::max(0.1f, std::min(distance, 1000.0f));

            lastX = nowX;
            lastY = nowY;
        }
        if (isDraggingLeft && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS){
            double nowX, nowY;
            glfwGetCursorPos(window, &nowX, &nowY);
            
            phi += (lastX - nowX) / width * M_PI;
            theta += (lastY - nowY) / height * M_PI;

            lastX = nowX;
            lastY = nowY;
        }

        if (isDraggingLeft && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            double nowX, nowY;
            glfwGetCursorPos(window, &nowX, &nowY);
            
            light_phi += (lastX - nowX) / width * M_PI;
            light_theta += (lastY - nowY) / height * M_PI;

            lastX = nowX;
            lastY = nowY;
        }

        float cosphi = cos(phi);
        float sinphi = sin(phi);
        float costheta = cos(theta);
        float sintheta = sin(theta);

        float x = distance * sintheta * sinphi;
        float y = distance * costheta;
        float z = distance * sintheta* cosphi;

        cy::Matrix4f translation = cy::Matrix4f::Identity();
        translation.AddTranslation({-x, -y, -z});

        cyMatrix4f r1 = cyMatrix4f::Identity().RotationY(-phi);
        cyMatrix4f r2 = cyMatrix4f::Identity().RotationX(-theta);
        
        rotation1.SetRotationY(phi);
        cy::Vec3f newaxis = { cosphi, 0, -sinphi };
        rotation2.SetRotation(newaxis, theta - M_PI / 2);
        cy::Matrix4f rotation = rotation2 * rotation1;
        rotation.Transpose();
        view = rotation * translation;

        proj.SetPerspective(1, 1, 0.1, 1000);

        glUseProgram(program);

        int modelloc = glGetUniformLocation(program, "model");
        glUniformMatrix4fv (modelloc, 1, GL_FALSE, &model.cell[0]);
        int viewloc = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewloc, 1, GL_FALSE, &view.cell[0]);
        int projloc = glGetUniformLocation(program, "proj");
        glUniformMatrix4fv(projloc, 1, GL_FALSE, &proj.cell[0]);
        int eyeloc = glGetUniformLocation(program, "in_eyepos");
        glUniform3f(eyeloc, x, y, z);

        int albedoloc = glGetUniformLocation(program, "in_albedo");
        glUniform3f(albedoloc, 0.5, 0.4, 0.8);
        int roughnessloc = glGetUniformLocation(program, "in_roughness");
        glUniform1f(roughnessloc, 500);

        cosphi = cos(light_phi);
        sinphi = sin(light_phi);
        costheta = cos(light_theta);
        sintheta = sin(light_theta);
        x = sintheta * sinphi;
        y = costheta;
        z = sintheta* cosphi;
        int lightloc = glGetUniformLocation(program, "in_lightdir");
        glUniform3f(lightloc, x, y, z);
        int radianceloc = glGetUniformLocation(program, "in_radiance");
        glUniform3f(radianceloc, 1.0, 1.0, 1.0);

        int usematloc = glGetUniformLocation(program, "hasTexture");
        glUniform1i(usematloc, (!diffusedata.empty() && !diffusedata.empty()) ? 1 : 0);

        if (!diffusedata.empty() && !diffusedata.empty()){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularID);
            GLuint sampler0 = glGetUniformLocation(program, "diffuse");
            glUniform1i(sampler0, 0);
            GLuint sampler1 = glGetUniformLocation(program, "specular");
            glUniform1i(sampler1, 0);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, mesh.NF() * 3, GL_UNSIGNED_INT, nullptr);
        //glDrawArrays(GL_TRIANGLES, 0, vertexbuffer.size());
        glBindVertexArray(0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}