#include "Camera.hpp"

namespace Sol {

static Camera sCamera;
Camera *Camera::instance() { return &sCamera; }

void Camera::set_time() {
    float time = Clock::instance()->set_time();
    delta_time = time / 1e9;
}

mat4 Camera::mat_view() { return lookAt(pos, pos + front, up); }
mat4 Camera::mat_proj() {
    float c_width = width <= 0 ? 1 : width;
    float c_height = height <= 0 ? 1 : height;
    return glm::perspective(glm::radians(fov), c_width / c_height, near_plane,
                       far_plane);
}
void Camera::move(Direction dir) {
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
void Camera::look(float xpos, float ypos) {
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
void Camera::zoom(float ypos) {
    fov -= ypos;
    if (fov < 20)
        fov = 20;
    if (fov > 90)
        fov = 90;
}
void Camera::update() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = normalize(front);

    right = glm::normalize(glm::cross(front, vec3(0, 1, 0)));
    up = glm::normalize(glm::cross(right, front));
}

} // namespace Sol
