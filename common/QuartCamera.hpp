#pragma once

#define GLM_FORCE_RADIANS
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Sol {

struct QuartCamera {
    static QuartCamera *instance();
    virtual ~QuartCamera() {}
    float window_width = 640;
    float window_height = 480;
    float xpos = 0;
    float ypos = 0;
    float yaw = 0;
    float pitch = 0.0f;
    float fov = 60.0f;
    float delta_time = 0;

    float speed = 3.0f;
    float sens = 0.3;

    glm::vec3 position = glm::vec3(0, 0, 10);

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    inline void updates() {
        float change = sens * delta_time;
        yaw += change * xpos;
        pitch += change * ypos;
    }
    inline glm::mat4 get_view() {

        // Reference:
        /* https://stackoverflow.com/questions/49609654/quaternion-based-first-person-view-camera
         */

        // FPS camera:  RotationX(pitch) * RotationY(yaw)
        glm::quat quat_pitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
        glm::quat quat_yaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
        // glm::quat qRoll = glm::angleAxis(roll,glm::vec3(0,0,1));

        // For a FPS camera we can omit roll
        glm::quat orientation = quat_pitch * quat_yaw;
        orientation = glm::normalize(orientation);
        glm::mat4 rotate = glm::mat4_cast(orientation);

        glm::mat4 translate = glm::mat4(1.0f);
        glm::vec3 eye(cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw));
        translate = glm::translate(translate, -eye);

        view = rotate * translate;
        return view;
    }
    inline glm::mat4 get_proj() {
        proj = glm::perspective(glm::radians(fov), window_width / window_height,
                                0.1f, 100.0f);
        return proj;
    }
    inline glm::mat4 update_and_calc_proj_view(float delta_time) {
        updates();
        return get_proj() * get_view();
    }

    inline void zoom(float zoom) { fov -= zoom; }
};

struct QuartFlyCamera : public QuartCamera {
    ~QuartFlyCamera() override;
};

struct QuartFpsCamera : public QuartCamera {
    ~QuartFpsCamera() override;
};

} // namespace Sol
