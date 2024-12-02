#pragma once

#include "pch.h"

#include "Camera.h"

enum class CameraType
{
    ORBITER,
    FREEFLY
};

class CameraSystem
{
public:
    CameraSystem(CameraType type = CameraType::ORBITER);

    void toggle_type();
    void type(CameraType type);

    Transform view();
    Transform projection();
    Transform projection(float width, float height, float fov = -1.0f);
    Transform viewport() const;

    void rotation(float mx, float my);
    void translation(float mx, float my, float dt, CameraMovement movement = CameraMovement::NONE);
    void move(float z, float dt);

    Point position() const;

    float& fov();

    inline bool is_orbiter() const { return m_type == CameraType::ORBITER; }   
    inline bool is_freefly() const { return m_type == CameraType::FREEFLY; }   

    CameraType type() const;

    Camera &freefly();
    Orbiter &orbiter();

private:
    Orbiter m_orbiter{};
    Camera m_freefly{};

    CameraType m_type{CameraType::ORBITER};
};
