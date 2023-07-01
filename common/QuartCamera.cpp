#include "Camera.hpp"

namespace Sol {

static Camera sCamera;

Camera *Camera::instance() { return &sCamera; }

} // namespace Sol
