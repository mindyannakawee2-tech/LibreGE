#include "Camera.hpp"

#include <algorithm>

namespace LibreGE {

Camera::Camera(
    float x,
    float y,
    float zoom
)
    : m_X(x),
      m_Y(y),
      m_Zoom(std::max(zoom, 0.01f)) {
}

void Camera::SetPosition(
    float x,
    float y
) {
    m_X = x;
    m_Y = y;
}

void Camera::Move(
    float dx,
    float dy
) {
    m_X += dx;
    m_Y += dy;
}

void Camera::SetZoom(
    float zoom
) {
    m_Zoom =
        std::max(
            zoom,
            0.01f
        );
}

float Camera::GetX() const {
    return m_X;
}

float Camera::GetY() const {
    return m_Y;
}

float Camera::GetZoom() const {
    return m_Zoom;
}

float Camera::WorldToScreenX(
    float worldX
) const {
    return
        (worldX - m_X) *
        m_Zoom;
}

float Camera::WorldToScreenY(
    float worldY
) const {
    return
        (worldY - m_Y) *
        m_Zoom;
}

float Camera::Scale(
    float value
) const {
    return
        value *
        m_Zoom;
}

}
