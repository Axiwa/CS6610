#include <iostream>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "lodepng.h"

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

    // =============================== Environment mapping ============================================================
    GLuint envprogram = glCreateProgram();
    GLuint envvert, envfrag;
    LoadShader("C:/Learn/CS6610/proj6/sampleenv.vert", envvert, GL_VERTEX_SHADER);
    LoadShader("C:/Learn/CS6610/proj6/sampleenv.frag", envfrag, GL_FRAGMENT_SHADER);
    glAttachShader(envprogram, envvert);
    glAttachShader(envprogram, envfrag);
    glLinkProgram(envprogram);

    bool status = check_program_status(envprogram, GL_LINK_STATUS);
    if (!status){
        exit(1);
    }

    GLuint envtex;
    glGenTextures(1, &envtex);

    VertexArray quad;

    // 1. load images
    std::vector<std::vector<unsigned char>> images_origin(6);
    std::vector<std::vector<unsigned char>> images(6);
    std::vector<std::pair<unsigned int, unsigned int>> image_size(6);
    lodepng::load_file(images_origin[0], asset("cubemap/cubemap_posx.png"));
    lodepng::load_file(images_origin[1], asset("cubemap/cubemap_negx.png"));
    lodepng::load_file(images_origin[2], asset("cubemap/cubemap_posy.png"));
    lodepng::load_file(images_origin[3], asset("cubemap/cubemap_negy.png"));
    lodepng::load_file(images_origin[4], asset("cubemap/cubemap_posz.png"));
    lodepng::load_file(images_origin[5], asset("cubemap/cubemap_negz.png"));
    for(int i = 0; i<6; i++){
        lodepng::decode(images[i], image_size[i].first, image_size[i].second, images_origin[i]);
    }
    
    // 2. Bind data
    glBindTexture(GL_TEXTURE_CUBE_MAP, envtex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    for(int i = 0; i<6; i++){
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 
            0,
            GL_RGBA,
            image_size[i].first,
            image_size[i].second,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            images[i].data()
        );
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // =============================== Framebuffer preparation =========================================================
    GLuint mirror;
    glGenFramebuffers(1, &mirror);
    glBindFramebuffer(GL_FRAMEBUFFER, mirror);
     
    GLuint renderTexture;
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mywindow.width, mywindow.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    GLuint depthbuffer;
    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mywindow.width, mywindow.height);

    glBindFramebuffer(GL_FRAMEBUFFER, mirror);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);

    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		std::cerr << "Frame buffer is not configured!" << std::endl;
		exit(1);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);    

    // =============================================================================================================== //
    // Load shaders for object
    GLuint objprogram = glCreateProgram();
    GLuint objvert, objfrag;
    LoadShader("C:/Learn/CS6610/proj6/obj.vert", objvert, GL_VERTEX_SHADER);
    LoadShader("C:/Learn/CS6610/proj6/obj.frag", objfrag, GL_FRAGMENT_SHADER);
    glAttachShader(objprogram, objvert);
    glAttachShader(objprogram, objfrag);
    glLinkProgram(objprogram);

    status = check_program_status(objprogram, GL_LINK_STATUS);
    if (!status){
        exit(1);
    }

    GLuint mirrorprogram = glCreateProgram();
    GLuint objvert2, objfrag2;
    LoadShader("C:/Learn/CS6610/proj6/obj_mirror.vert", objvert2, GL_VERTEX_SHADER);
    LoadShader("C:/Learn/CS6610/proj6/obj_mirror.frag", objfrag2, GL_FRAGMENT_SHADER);
    glAttachShader(mirrorprogram, objvert2);
    glAttachShader(mirrorprogram, objfrag2);
    glLinkProgram(mirrorprogram);

    status = check_program_status(mirrorprogram, GL_LINK_STATUS);
    if (!status){
        exit(1);
    }

    // Load meshes
    cyTriMesh mesh;
    mesh.LoadFromFileObj(asset("teapot/teapot.obj").c_str());

    // Bind Vertex Array
    VertexArray obj;
    obj.Create(mesh, objprogram);

    mesh.ComputeBoundingBox();
    cyVec3f modelpos = (mesh.GetBoundMax() + mesh.GetBoundMin()) / 2;

    UniformBufferManager umgr;

    // Transform of model
    cyMatrix4f model = cyMatrix4f::Identity();
    model.Zero();
    model.cell[2] = 1;
    model.cell[4] = 1;
    model.cell[9] = 1;
    model.cell[15] = 1;

    // Light
    DirectionalLightBlock directional;
    float philight = 0, thetalight = M_PI/ 2;
    directional.direction = cyVec4f(-sin(thetalight) * sin(philight), -cos(thetalight), -sin(thetalight) * cos(philight), 0);
    directional.radiance = cyVec4f(1, 1, 1, 0);
    umgr.UpdateLightUniformBlock<DirectionalLightBlock>(directional, 0);

    // Material
    MaterialBlock mat;
    mat.ambient = cyVec4f(0.01);
    mat.albedo = cyVec4f(1, 1, 1, 0);
    mat.specular = cyVec4f(0, 1, 1, 0);
    mat.shininess = 50;
    umgr.UpdateMaterialUniformBlock<MaterialBlock>(mat, 0);

    // Mirror camera block
    GLuint mirrorCamera;
    glGenBuffers(1, &mirrorCamera);
    glBindBuffer(GL_UNIFORM_BUFFER, mirrorCamera);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, mirrorCamera);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    CameraBlock mirrorcam;

    // Plane
    GLuint planeprogram = glCreateProgram();
    GLuint planevert, planefrag;
    LoadShader("C:/Learn/CS6610/proj6/obj_plane.vert", planevert, GL_VERTEX_SHADER);
    LoadShader("C:/Learn/CS6610/proj6/obj_plane.frag", planefrag, GL_FRAGMENT_SHADER);
    glAttachShader(planeprogram, planevert);
    glAttachShader(planeprogram, planefrag);
    glLinkProgram(planeprogram);

    status = check_program_status(planeprogram, GL_LINK_STATUS);
    if (!status){
        exit(1);
    }

    VertexArray plane;
    cyTriMesh planemesh;
    planemesh.LoadFromFileObj(asset("subdevidedplane.obj").c_str());
    plane.Create(planemesh, planeprogram);
    cyMatrix4f planemodel = cyMatrix4f::Identity();
    cyVec3f planepos = {0, 0, 0};
    cyVec3f normal = cyVec3f(0,1,0);

    double deltax, deltay;
    while(!glfwWindowShouldClose(mywindow.window)){

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

        // mirror plane position
        if (glfwGetKey(mywindow.window, GLFW_KEY_UP) == GLFW_PRESS){
            planemodel.AddTranslation({0, 0.1, 0});
            planepos.y += 0.1f;
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_DOWN) == GLFW_PRESS){
            planemodel.AddTranslation({0, -0.1, 0});
            planepos.y -= 0.1f;
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_LEFT) == GLFW_PRESS){
            cyMatrix4f rotation = cyMatrix4f::RotationX(M_PI / 1800);
            planemodel =  planemodel * rotation;
            normal = cyMatrix3f(rotation).GetTranspose().GetInverse() * normal;
        }
        else if (glfwGetKey(mywindow.window, GLFW_KEY_RIGHT) == GLFW_PRESS){
            cyMatrix4f rotation = cyMatrix4f::RotationX(-M_PI / 1800);
            planemodel = planemodel  * rotation;
            normal = cyMatrix3f(rotation).GetTranspose().GetInverse() * normal;
        }

        mywindow.scene_camera.on_distance_change();

        umgr.UpdateCameraUniformBlock<cyMatrix4f>(mywindow.scene_camera.get_view(), offsetof(CameraBlock, view));
        umgr.UpdateCameraUniformBlock<cyMatrix4f>(mywindow.scene_camera.get_projection(), offsetof(CameraBlock, projection));
        cyVec4f tmp = cyVec4f(mywindow.scene_camera.get_pos(), 1);
        umgr.UpdateCameraUniformBlock<cyVec4f>(tmp, offsetof(CameraBlock, eyepos));

        // mirrorcam to plane normal; plane's normal is (0,1,0), a point on the plane is planepos
        cyVec3f campos = mywindow.scene_camera.get_pos();
        float phi = mywindow.scene_camera._phi + M_PI;
        float theta = M_PI / 2 - mywindow.scene_camera._theta;
		float sinphi = sin(phi);
		float sintheta = sin(theta);
		float cosphi = cos(phi);
		float costheta = cos(theta);
		float eyex = sintheta * sinphi;
		float eyey = costheta;
		float eyez = sintheta * cosphi;
        cyVec3f up = cyVec3f(eyex, eyey, eyez);
        up = up - 2 * normal.Dot(up) * normal;

        cyVec3f eyedir = cyVec3f(0, 0, 0) - campos;
        cyVec3f neweyedir = eyedir - 2 * normal.Dot(eyedir) * normal;
        float distance = (campos - planepos).Dot(normal);
        campos = campos - normal * distance * 2;

        mirrorcam.projection = mywindow.scene_camera.get_projection();
        mirrorcam.eyepos = cyVec4f(campos, 1);
        
        mirrorcam.view = cyMatrix4f::View(campos, neweyedir + campos, up);
        cyMatrix4f one = cyMatrix4f::Identity();
        one.cell[0] = -1;
        mirrorcam.view = one * mirrorcam.view;

		GLint origFB;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

        // Draw Object to Framebuffer
        glUseProgram(mirrorprogram);

        // 1. Camera
		glBindBuffer(GL_UNIFORM_BUFFER, mirrorCamera);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraBlock), &mirrorcam);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // 2. I don't know why GL_CULL_FACE FRONT
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mirror);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glViewport(0, 0, mywindow.width, mywindow.height);
		glClearColor(0, 1, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int modellocobjinmirror = glGetUniformLocation(mirrorprogram, "model");
        glUniformMatrix4fv(modellocobjinmirror, 1, GL_FALSE, &model.cell[0]);
        int planeposloc = glGetUniformLocation(mirrorprogram, "planepos");
        int planenormalloc = glGetUniformLocation(mirrorprogram, "planenormal");
        glUniform3f(planeposloc, planepos.x, planepos.y, planepos.z);
        glUniform3f(planenormalloc, normal.x, normal.y, normal.z);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envtex);
        GLuint sampler0 = glGetUniformLocation(mirrorprogram, "envTex");
        glUniform1i(sampler0, 0);
        obj.Draw();

        // Draw Object to Screen
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB);
		glClearColor(0.2, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, mywindow.width, mywindow.height);
		glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glUseProgram(objprogram);
        int modellocobj = glGetUniformLocation(objprogram, "model");
        glUniformMatrix4fv(modellocobj, 1, GL_FALSE, &model.cell[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envtex);
        sampler0 = glGetUniformLocation(objprogram, "envTex");
        glUniform1i(sampler0, 0);
        obj.Draw();

        // Draw Plane
        glUseProgram(planeprogram);
        int modellocplane = glGetUniformLocation(planeprogram, "model");
        glUniformMatrix4fv(modellocplane, 1, GL_FALSE, &planemodel.cell[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envtex);
        sampler0 = glGetUniformLocation(planeprogram, "envTex");
        glUniform1i(sampler0, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        GLuint sampler1 = glGetUniformLocation(planeprogram, "mirror");
        glUniform1i(sampler1, 1);
        plane.Draw();

        // Draw Enviornment
        // 0. Disable depth write
        glDisable(GL_DEPTH_WRITEMASK);
        glDisable(GL_CULL_FACE);
        // 1. Calculate quad position
        cyMatrix4f clipmatrix = mywindow.scene_camera.get_projection() * mywindow.scene_camera.get_view();
        cyMatrix4f invclipmatrix = clipmatrix.GetInverse();
        cyVec4f x = invclipmatrix * cyVec4f(-1, -1, 0.999, 1);
        cyVec4f y = invclipmatrix * cyVec4f(3, -1, 0.999, 1);
        cyVec4f z = invclipmatrix * cyVec4f(-1, 3, 0.999, 1);
        quad.CreateTriangle(x.XYZ() / x.w, y.XYZ()/y.w, z.XYZ()/z.w, envprogram);

        // 2. Bind texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        GLuint sampler2 = glGetUniformLocation(envprogram, "envTex");
        glUniform1i(sampler2, 2);

        // 3. Draw
        glUseProgram(envprogram);
        quad.Draw();

		glfwSwapBuffers(mywindow.window);
		glfwPollEvents();
    }
}