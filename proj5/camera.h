#pragma once

#include "cy/cyVector.h"
#include "cy/cyMatrix.h"

#define M_PI 3.1415926

struct Camera {
    bool isPersp = true;

    float fov = M_PI / 2;
    float near = 0.1;
    float far = 10000;

    cyMatrix4f get_world_to_camera(){
		viewTex = rotation * translation;
        return viewTex;
    }

private:
    cyMatrix4f viewTex = cyMatrix4f::Identity();
    cy::Vec3f pos = {50, 50, 50};
    cy::Matrix4f translation;
    cy::Matrix4f rotation;
    cy::Matrix4f projection;

    void on_param_change(float fov, float aspect, float near, float far){
        projection.SetPerspective(fov, aspect, near, far);
    }

    void on_pos_change(cyVec3f newpos){
        pos = newpos;
        translation = cyMatrix4f::Translation({ -pos.x, -pos.y, -pos.z });
    }

    void on_rotation_change(){

    }
};