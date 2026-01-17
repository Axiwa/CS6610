#pragma once

#include "cy/cyVector.h"
#include "cy/cyMatrix.h"

#define M_PI 3.1415926

struct orbit_camera {
    float _phi = 0;
    float _theta = M_PI / 2;
    float _dist = 50;
    float _fov = M_PI / 2, _near = 0.1, _far = 10000;

    orbit_camera(float fov, float aspect, float near, float far):
    _fov(fov),
    _near(near),
    _far(far)
    {
		update_matrix();
        projection.SetPerspective(fov, aspect, near, far);
    }

    orbit_camera(float fov, float aspect, float near, float far, float phi, float theta, float dist):
    _fov(fov),
    _near(near),
    _far(far),
    _phi(phi),
    _theta(theta),
    _dist(dist)
    {
		update_matrix();
        projection.SetPerspective(fov, aspect, near, far);
    }

    cyVec3f get_pos(){
		float sinphi = sin(_phi);
		float sintheta = sin(_theta);
		float cosphi = cos(_phi);
		float costheta = cos(_theta);

		float eyex = sintheta * sinphi;
		float eyey = costheta;
		float eyez = sintheta * cosphi;
        cyVec3f pos = _dist * cyVec3f({eyex, eyey, eyez});

        return pos;
    }

    cyMatrix4f& get_view(){
        return view;
    }

    cyMatrix4f& get_projection(){
		return projection;
    }

    void on_rotation_change(float delta_phi, float delta_theta){
        _phi -= delta_phi;
        _theta -= delta_theta;
        update_matrix();
    }

    void on_distance_change(float new_distance){
        _dist = new_distance;
        update_matrix();
    }

    void on_distance_change(){
        update_matrix();
    }

    void on_param_change(float fov, float aspect, float near, float far){
        _fov = fov;
        _near = near;
        _far = far;
        projection.SetPerspective(fov, aspect, near, far);
    }

    void on_aspect_change(float aspect){
        projection.SetPerspective(_fov, aspect, _near, _far);
    }

private:
    cyMatrix4f view = cyMatrix4f::Identity();
    cy::Vec3f pos = {50, 50, 50};
    cy::Matrix4f translation = cyMatrix4f::Identity();
    cy::Matrix4f rotation = cyMatrix4f::Identity();
    cy::Matrix4f projection = cyMatrix4f::Identity();

    void on_pos_change(cyVec3f newpos){
        pos = newpos;
        translation = cyMatrix4f::Translation({ -pos.x, -pos.y, -pos.z });
    }

    void update_matrix(){
		float sinphi = sin(_phi);
		float sintheta = sin(_theta);
		float cosphi = cos(_phi);
		float costheta = cos(_theta);

		float eyex = sintheta * sinphi;
		float eyey = costheta;
		float eyez = sintheta * cosphi;
        cyVec3f pos = _dist * cyVec3f({-eyex, -eyey, -eyez});

        translation = cyMatrix4f::Translation({pos.x, pos.y, pos.z});

        cyMatrix4f r1 = cyMatrix4f::RotationY(_phi);
        cyVec3f newaxis = {cosphi, 0, -sinphi};
        cyMatrix4f r2 = cyMatrix4f::Rotation(newaxis, _theta - M_PI / 2);
        rotation = r2 * r1;
        rotation.Transpose();
        view = rotation * translation;
    }

};