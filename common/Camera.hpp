#pragma once

#define GLM_FORCE_RADIANS

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Clock.hpp"

namespace Sol {

using namespace glm;

enum Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
};

struct Camera {

    static Camera *instance();

    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 5.0;
    float sens = 4.0;
    float fov = 60.0f;
    float delta_time = 0;
    float near_plane = 0.1f;
    float far_plane = 100.0f;

    float height = 480.0f;
    float width = 640.0f;

    vec3 pos = vec3(0.0, 0.0, 3.0);
    vec3 front;
    vec3 right;
    vec3 up;

    void set_time() {
        float time = Clock::instance()->set_time();
        delta_time = time / 1e9;
    }

    mat4 mat_view() { return lookAt(pos, pos + front, up); }
    mat4 mat_proj() {
        float c_width = width <= 0 ? 1 : width;
        float c_height = height <= 0 ? 1 : height;
        return perspective(radians(fov), c_width / c_height, near_plane,
                           far_plane);
    }
    void move(Direction dir) {
        float vel = speed * delta_time;
        if (dir == FORWARD)
            pos += front * vel;
        if (dir == BACKWARD)
            pos -= front * vel;
        if (dir == LEFT)
            pos -= right * vel;
        if (dir == RIGHT)
            pos += right * vel;
    }
    void look(float xpos, float ypos) {
        float new_sens = sens * delta_time;
        xpos *= new_sens;
        ypos *= new_sens;
        yaw += xpos;
        pitch -= ypos;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        update();
    }
    void zoom(float ypos) {
        fov -= ypos;
        if (fov < 20)
            fov = 20;
        if (fov > 90)
            fov = 90;
    }
    void update() {
        front.x = cos(radians(yaw)) * cos(radians(pitch));
        front.y = sin(radians(pitch));
        front.z = sin(radians(yaw)) * cos(radians(pitch));
        front = normalize(front);

        right = normalize(cross(front, vec3(0, 1, 0)));
        up = normalize(cross(right, front));
    }
};

} // namespace Sol
