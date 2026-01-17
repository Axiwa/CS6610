#include <iostream>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "lodepng.h"

#include "cy/cyTriMesh.h"
#include "window.h"
#include "shaderloader.h"
#include "vertexarray.h"
#include "orbitcamera.h"

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
    myWindow mywindow(1024, 1024, "proj7");
    glfwMakeContextCurrent(mywindow.window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // load mesh
    cyTriMesh mesh;
    if (!mesh.LoadFromFileObj(asset("teapot/teapot.obj").c_str())){
        std::cerr<<"Failed to load object!"<<std::endl;
        exit(1);
    }
    cyMatrix4f meshmodel;
    meshmodel.Zero();
    meshmodel.cell[2] = 1;
    meshmodel.cell[4] = 1;
    meshmodel.cell[9] = 1;
    meshmodel.cell[15] = 1;
    // meshmodel.AddTranslation({0, -3, 0});

    CameraBlock camblock;
    UniformBufferManager objmgr;

    // load plane -- plane will be loaded directly

    // load shader
    GLuint objprogram = glCreateProgram();
    GLuint vertshader, fragshader;
    if (!LoadShader("C:/Learn/CS6610/proj7/obj.vert", vertshader, GL_VERTEX_SHADER)) {std::cerr<<"Load vertex shader failed!"<<std::endl; exit(1);}
    if (!LoadShader("C:/Learn/CS6610/proj7/obj.frag", fragshader, GL_FRAGMENT_SHADER)) {std::cerr<<"Load fragment shader failed!"<<std::endl; exit(1);}

    glAttachShader(objprogram, vertshader);
    glAttachShader(objprogram, fragshader);
    glLinkProgram(objprogram);
    if (!check_program_status(objprogram, GL_LINK_STATUS)){std::cerr<<"Link object program failed!"<<std::endl; exit(1); }
    // bind vertexbuffer to program
    VertexArray objarray;
    objarray.Create(mesh, objprogram);

    // shadow program
    GLuint objshadowprogram = glCreateProgram();
    if (!LoadShader("C:/Learn/CS6610/proj7/shadow.vert", vertshader, GL_VERTEX_SHADER)) {std::cerr<<"Load vertex shader failed!"<<std::endl; exit(1);}
    if (!LoadShader("C:/Learn/CS6610/proj7/shadow.frag", fragshader, GL_FRAGMENT_SHADER)) {std::cerr<<"Load fragment shader failed!"<<std::endl; exit(1);}
    glAttachShader(objshadowprogram, vertshader);
    glAttachShader(objshadowprogram, fragshader);
    glLinkProgram(objshadowprogram);
    if (!check_program_status(objshadowprogram, GL_LINK_STATUS)){std::cerr<<"Link object program failed!"<<std::endl; exit(1); }

    // bind vertexbuffer to program
    VertexArray objshadowarray;
    objshadowarray.Create(mesh, objshadowprogram);

    // Light
    LightBlock lightblock;
    float philight = M_PI, thetalight = M_PI/ 3;
    lightblock.direction = cyVec4f(-sin(thetalight) * sin(philight), -cos(thetalight), -sin(thetalight) * cos(philight), 0); // ↓
    lightblock.radiance = cyVec4f(800, 800, 800, 0);
    lightblock.pos = -lightblock.direction * 40;
    lightblock.angle = cyVec4f(M_PI / 4, M_PI / 4, M_PI / 4, M_PI / 4); // spot
    lightblock.type = 1;
    objmgr.UpdateLightUniformBlock<LightBlock>(lightblock, 0);

    MaterialBlock matblock;
    matblock.albedo = cyVec4f(1, 1, 1, 1);
    matblock.ambient = cyVec4f(0.1, 0.1, 0.1, 1);
    matblock.shininess.x = 30;
    matblock.specular = cyVec4f(1, 0, 0, 1);
    objmgr.UpdateMaterialUniformBlock<MaterialBlock>(matblock, 0);

    GLuint planeprogram = glCreateProgram();
    GLuint vertshader2, fragshader2;
    if (!LoadShader("C:/Learn/CS6610/proj7/obj.vert", vertshader2, GL_VERTEX_SHADER)) {std::cerr<<"Load vertex shader failed!"<<std::endl; exit(1);}
    if (!LoadShader("C:/Learn/CS6610/proj7/obj.frag", fragshader2, GL_FRAGMENT_SHADER)) {std::cerr<<"Load fragment shader failed!"<<std::endl; exit(1);}
    glAttachShader(planeprogram, vertshader2);
    glAttachShader(planeprogram, fragshader2);
    glLinkProgram(planeprogram);
    if (!check_program_status(planeprogram, GL_LINK_STATUS)){
        std::cerr<<"Failed to link plane program!"<<std::endl;
        exit(1);
    }
    VertexArray planearray;
    cyTriMesh planemesh;
    planemesh.LoadFromFileObj(asset("subdevidedplane.obj").c_str());
    planearray.Create(planemesh, planeprogram);
    cyMatrix4f planemodel = cyMatrix4f::Identity();

    GLuint planeshadowprogram = glCreateProgram();
    if (!LoadShader("C:/Learn/CS6610/proj7/shadow.vert", vertshader, GL_VERTEX_SHADER)) {std::cerr<<"Load vertex shader failed!"<<std::endl; exit(1);}
    if (!LoadShader("C:/Learn/CS6610/proj7/shadow.frag", fragshader, GL_FRAGMENT_SHADER)) {std::cerr<<"Load fragment shader failed!"<<std::endl; exit(1);}
    glAttachShader(planeshadowprogram, vertshader);
    glAttachShader(planeshadowprogram, fragshader);
    glLinkProgram(planeshadowprogram);
    if (!check_program_status(planeshadowprogram, GL_LINK_STATUS)){
        std::cerr<<"Failed to link shadow program!"<<std::endl;
        exit(1);
    }
    VertexArray planeshadowarray;
    planeshadowarray.Create(planemesh, planeshadowprogram);

    // shadow camera
    CameraBlock lightshadow;
    lightshadow.eyepos = lightblock.pos;
    lightshadow.view = cyMatrix4f::View(lightblock.pos.XYZ(), lightblock.pos.XYZ() + lightblock.direction.XYZ(), {0, 1, 0});
    lightshadow.projection = cyMatrix4f::Perspective(M_PI / 2, 1, 20.f, 100.f);
    GLuint shadowcamerabuffer;
    glGenBuffers(1, &shadowcamerabuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, shadowcamerabuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, shadowcamerabuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // shadow framebuffer
    GLuint shadowframebuffer;
    glGenFramebuffers(1, &shadowframebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowframebuffer);

    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 2.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    int shadWidth = 1024, shadHeight = 1024;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadWidth, shadHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while(!glfwWindowShouldClose(mywindow.window)){

		GLint origFB;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

        double deltax, deltay;
        mywindow.deltaMouse(deltax, deltay);
        float deltaPhi = deltax / mywindow.width * M_PI * 2;
        float deltaTheta = deltay / mywindow.height * M_PI;

        if (mywindow.is_dragging_left){
            mywindow.scene_camera.on_rotation_change(deltaPhi, deltaTheta);
        }
        
        if (mywindow.is_dragging_right){
            philight += deltaPhi;
            thetalight += deltaTheta;
            if (thetalight < 0){
                thetalight = 0;
            }
            if (thetalight > M_PI / 2){
                thetalight = M_PI / 2;
            }
            lightblock.direction = cyVec4f(-sin(thetalight) * sin(philight), -cos(thetalight), -sin(thetalight) * cos(philight), 0);
            objmgr.UpdateLightUniformBlock(lightblock.direction, offsetof(LightBlock, direction));
        }
        
        if (glfwGetKey(mywindow.window, GLFW_KEY_UP) == GLFW_PRESS){
            lightblock.pos.y += 0.1f;
            objmgr.UpdateLightUniformBlock(lightblock.pos, offsetof(LightBlock, pos));
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_DOWN) == GLFW_PRESS){
            lightblock.pos.y -= 0.1f;
            objmgr.UpdateLightUniformBlock(lightblock.pos, offsetof(LightBlock, pos));
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_LEFT) == GLFW_PRESS){
            lightblock.pos.z += 0.1f;
            objmgr.UpdateLightUniformBlock(lightblock.pos, offsetof(LightBlock, pos));
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_RIGHT) == GLFW_PRESS){
            lightblock.pos.z -= 0.1f;
            objmgr.UpdateLightUniformBlock(lightblock.pos, offsetof(LightBlock, pos));
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_W) == GLFW_PRESS){
            lightblock.pos.x += 0.1f;
            objmgr.UpdateLightUniformBlock(lightblock.pos, offsetof(LightBlock, pos));
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_S) == GLFW_PRESS){
            lightblock.pos.x -= 0.1f;
            objmgr.UpdateLightUniformBlock(lightblock.pos, offsetof(LightBlock, pos));
        }

        lightshadow.view = cyMatrix4f::View(lightblock.pos.XYZ(), lightblock.pos.XYZ() + lightblock.direction.XYZ(), {0, 1, 0});
        glBindBuffer(GL_UNIFORM_BUFFER, shadowcamerabuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraBlock), &lightshadow);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        mywindow.scene_camera.on_distance_change();

        camblock.eyepos = cyVec4f(mywindow.scene_camera.get_pos(), 1);
        camblock.projection = mywindow.scene_camera.get_projection();
        camblock.view = mywindow.scene_camera.get_view();
        objmgr.UpdateCameraUniformBlock<CameraBlock>(camblock, 0);

        // 1. Draw shadow map
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowframebuffer);
        glViewport(0, 0, shadWidth, shadHeight);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1, 0, 0.2, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(objshadowprogram);
        int modelshadowloc = glGetUniformLocation(objshadowprogram, "model");
        glUniformMatrix4fv(modelshadowloc, 1, GL_FALSE, &meshmodel.cell[0]);
        objshadowarray.Draw();

        glUseProgram(planeshadowprogram);
        int planeshadowloc = glGetUniformLocation(planeshadowprogram, "model");
        glUniformMatrix4fv(planeshadowloc, 1, GL_FALSE, &planemodel.cell[0]);    
        planeshadowarray.Draw();

        // 2. Draw object
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB);
        glViewport(0, 0, mywindow.width, mywindow.height);
        glClearColor(0.1, 0, 0.2, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(objprogram);
        int modelloc = glGetUniformLocation(objprogram, "model");
        glUniformMatrix4fv(modelloc, 1, GL_FALSE, &meshmodel.cell[0]);
        
        glActiveTexture(GL_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        GLuint sampler = glGetUniformLocation(objprogram, "shadow");
        glUniform1i(sampler, 0);

        objarray.Draw();

        glUseProgram(planeprogram);
        int planeloc = glGetUniformLocation(planeprogram, "model");
        glUniformMatrix4fv(planeloc, 1, GL_FALSE, &planemodel.cell[0]); 
        
        GLuint sampler2 = glGetUniformLocation(planeprogram, "shadow");
        glUniform1i(sampler2, 0);
        planearray.Draw();
		
		glfwSwapBuffers(mywindow.window);
		glfwPollEvents();
    }
    
}