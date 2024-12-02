#pragma once

#include "pch.h"

enum CameraMovement
{
    NONE=0,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float MOVEMENT_SPEED = 15.0f;
const float ROTATION_SPEED = 0.1f;
const float ZOOM = 45.0f;

// Free-fly camera
class Camera
{
public:
    Camera(Point position = {0.0f, 0.0f, 0.0f}, float w_width = 1.0f, float w_height = 1.0f, float fov = 45.0f, float yaw = YAW, float pitch = PITCH);

    Transform view();
    Transform projection();
    Transform projection(float width, float height, float fov);
    Transform viewport() const;

    void translation(CameraMovement direction, float dt);
    void rotation(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void move(float yoffset, float dt);

    inline Point& position() { return m_position; }
    inline Point position() const { return m_position; }

    inline float& rotation_speed() { return m_rotation_speed; }
    inline float& movement_speed() { return m_movement_speed; }
    inline float& fov() { return m_fov; }

private:
    void update();

    Point m_position{0.0f, 0.0f, 0.0f};
    Vector m_front{0.0f, 0.0f, -1.0f};
    Vector m_up{0.0f, 1.0f, 0.0f};
    Vector m_right{1.0f, 0.0f, 0.0f};
    Vector m_world_up{0.0f, 1.0f, 0.0f};

    // euler Angles
    float m_yaw{YAW};
    float m_pitch{PITCH};

    float m_movement_speed{MOVEMENT_SPEED};
    float m_rotation_speed{ROTATION_SPEED};
    float m_zoom{ZOOM};

    float m_window_width{1.0f};
    float m_window_height{1.0f};
    float m_fov{45.0f};
};
