#pragma once

#define GLM_FORCE_RADIANS

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Clock.hpp"

namespace Sol {

typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat4 mat4;

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
    float sens = 4.00;
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

    void set_time();

    mat4 mat_view(); 
    mat4 mat_proj(); 
    void move(Direction dir); 
    void look(float xpos, float ypos); 
    void zoom(float ypos); 
    void update(); 
};

} // namespace Sol
