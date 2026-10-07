#pragma once

namespace LibreGE {

class Camera {
public:
    Camera(
        float x = 0.0f,
        float y = 0.0f,
        float zoom = 1.0f
    );

    void SetPosition(
        float x,
        float y
    );

    void Move(
        float dx,
        float dy
    );

    void SetZoom(
        float zoom
    );

    float GetX() const;
    float GetY() const;
    float GetZoom() const;

    float WorldToScreenX(
        float worldX
    ) const;

    float WorldToScreenY(
        float worldY
    ) const;

    float Scale(
        float value
    ) const;

private:
    float m_X = 0.0f;
    float m_Y = 0.0f;

    float m_Zoom = 1.0f;
};

}
