#include "Camera.h"

Camera::Camera(Point position, float w_width, float w_height, float fov, float yaw, float pitch) : m_position(position), m_yaw(yaw), m_pitch(pitch)
{
    update();
}

Transform Camera::view()
{
    return Lookat(m_position, m_position + m_front, m_up);
}

Transform Camera::projection()
{
    return Perspective(m_fov, m_window_width / m_window_height, 0.1f, 100.0f);
}

Transform Camera::projection(float width, float height, float fov)
{
    m_window_width = width;
    m_window_height = height;
    m_fov = fov;
    return projection();
}

void Camera::translation(CameraMovement direction, float dt)
{
    float velocity = m_movement_speed * dt;
    switch (direction)
    {
    case CameraMovement::FORWARD:
        m_position = m_position + m_front * velocity;
        break;
    case CameraMovement::BACKWARD:
        m_position = m_position - m_front * velocity;
        break;
    case CameraMovement::LEFT:
        m_position = m_position - m_right * velocity;
        break;
    case CameraMovement::RIGHT:
        m_position = m_position + m_right * velocity;
        break;
    case CameraMovement::UP:
        m_position = m_position + m_up * velocity;
        break;
    case CameraMovement::DOWN:
        m_position = m_position - m_up * velocity;
        break;
    }
}

void Camera::rotation(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= m_rotation_speed;
    yoffset *= m_rotation_speed;

    m_yaw += xoffset;
    m_pitch -= yoffset;

    if (constrainPitch)
    {
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    }

    update();
}

void Camera::move(float yoffset, float dt)
{
    float velocity = m_movement_speed * yoffset * dt;
    m_position = m_position + m_front * velocity;
}

void Camera::update()
{
    Vector front;
    front.x = cos(radians(m_yaw)) * cos(radians(m_pitch));
    front.y = sin(radians(m_pitch));
    front.z = sin(radians(m_yaw)) * cos(radians(m_pitch));
    m_front = normalize(front);
    m_right = normalize(cross(m_front, m_world_up));
    m_up = normalize(cross(m_right, m_front));
}
