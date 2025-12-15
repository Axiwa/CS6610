#include <iostream>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "orbitcamera.h"
#include "rendertexture.h"
#include "shaderloader.h"
#include "vertexarray.h"
#include "window.h"

static std::string asset(const std::string& p)
{
    return std::string(ASSET_ROOT) + p;
}

bool check_program_status(GLuint obj, GLenum type){
	int success;
	int maxLength;
	glGetProgramiv(obj, type, (int*)&success);
	if (success == GL_FALSE) {
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, (int*)&maxLength);

		char* infoLog = (char*)malloc(maxLength);
		glGetProgramInfoLog(obj, maxLength, &maxLength, infoLog);
		std::cout << infoLog << std::endl;
		free(infoLog);
		return false;
	}
    return true;
}


int main(){
    myWindow mywindow(1024, 1024, "proj 6");
    glfwMakeContextCurrent(mywindow.window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Load shaders
    GLuint objprogram = glCreateProgram();
    GLuint objvert, objfrag;
    LoadShader("D:/Learn/CS6610/proj6/obj.vert", objvert, GL_VERTEX_SHADER);
    LoadShader("D:/Learn/CS6610/proj6/obj.frag", objfrag, GL_FRAGMENT_SHADER);
    glAttachShader(objprogram, objvert);
    glAttachShader(objprogram, objfrag);
    glLinkProgram(objprogram);

    bool status = check_program_status(objprogram, GL_LINK_STATUS);
    if (!status){
        exit(1);
    }

    // Load meshes
    cyTriMesh mesh;
    mesh.LoadFromFileObj(asset("teapot/teapot.obj").c_str());

    VertexArray obj;
    obj.Create(mesh, objprogram);

    // Transform
    cyMatrix4f model = cyMatrix4f::Identity();
    model.Zero();
    model.cell[2] = 1;
    model.cell[4] = 1;
    model.cell[9] = 1;
    model.cell[15] = 1;

    UniformBufferManager umgr;
    umgr.UpdateCameraUniformBlock<cyMatrix4f>(model, 0);

    DirectionalLightBlock directional;
    float philight = 0, thetalight = M_PI/ 2;

    directional.direction = cyVec4f(-sin(thetalight) * sin(philight), -cos(thetalight), -sin(thetalight) * cos(philight), 0);
    directional.radiance = cyVec4f(1, 1, 1, 0);
    umgr.UpdateLightUniformBlock<DirectionalLightBlock>(directional, 0);

    MaterialBlock mat;
    mat.ambient = cyVec4f(0.01);
    mat.albedo = cyVec4f(1, 1, 1, 0);
    mat.specular = cyVec4f(0, 0, 1, 0);
    mat.shininess = 500;
    umgr.UpdateMaterialUniformBlock<MaterialBlock>(mat, 0);

    double deltax, deltay;
    while(!glfwWindowShouldClose(mywindow.window)){
		glClearColor(0.2, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

        mywindow.deltaMouse(deltax, deltay);
        float deltaPhi = deltax / mywindow.width * M_PI * 2;
        float deltaTheta = deltay / mywindow.height * M_PI;
        if (mywindow.is_dragging_left) {
            mywindow.scene_camera.on_rotation_change(deltaPhi, deltaTheta);
        }
        if (mywindow.is_dragging_right){
            philight += deltaPhi;
            thetalight += deltaTheta;
            directional.direction = cyVec4f(-sin(thetalight) * sin(philight), -cos(thetalight), -sin(thetalight) * cos(philight), 0);
            umgr.UpdateLightUniformBlock<cyVec4f>(directional.direction, offsetof(DirectionalLightBlock, direction));
        }

        mywindow.scene_camera.on_distance_change();

        umgr.UpdateCameraUniformBlock<cyMatrix4f>(mywindow.scene_camera.get_view(), offsetof(CameraBlock, view));
        umgr.UpdateCameraUniformBlock<cyMatrix4f>(mywindow.scene_camera.get_projection(), offsetof(CameraBlock, projection));
        umgr.UpdateCameraUniformBlock<cyVec3f>(mywindow.scene_camera.get_pos(), offsetof(CameraBlock, eyepos));
        std::cout << mywindow.scene_camera.get_pos().x <<" " << mywindow.scene_camera.get_pos().y <<" "<< mywindow.scene_camera.get_pos().z << std::endl;

        glUseProgram(objprogram);
        obj.Draw();

		glfwSwapBuffers(mywindow.window);
		glfwPollEvents();
    }
}