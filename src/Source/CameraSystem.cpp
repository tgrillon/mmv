#include "CameraSystem.h"

CameraSystem::CameraSystem(CameraType type) : m_type(type), m_freefly(), m_orbiter()
{
}

void CameraSystem::toggle_type()
{
    switch (m_type)
    {
    case CameraType::ORBITER:
        m_type = CameraType::FREEFLY;
        break;

    case CameraType::FREEFLY:
        m_type = CameraType::ORBITER;
        break;
    }
}

void CameraSystem::type(CameraType type)
{
    m_type = type;
}

Transform CameraSystem::view()
{
    if (is_orbiter())
        return m_orbiter.view();
    else
        return m_freefly.view();
}

Transform CameraSystem::projection()
{
    if (m_type == CameraType::ORBITER)
        return m_orbiter.projection();
    else
        return m_freefly.projection();
}

Transform CameraSystem::projection(float width, float height, float fov)
{
    if (m_type == CameraType::ORBITER)
        return m_orbiter.projection(width, height, fov < 0.0f ? m_orbiter.fov() : fov);
    else
        return m_freefly.projection(width, height, fov < 0.0f ? m_freefly.fov() : fov);
}

void CameraSystem::rotation(float mx, float my)
{
    if (is_orbiter())
        m_orbiter.rotation(mx, my);
    else 
        m_freefly.rotation(mx, my);
    
}

void CameraSystem::translation(float mx, float my, float dt, CameraMovement movement)
{
    if (is_orbiter())
        m_orbiter.translation(mx, my);
    else if (movement != CameraMovement::NONE)
        m_freefly.translation(movement, dt);
}

void CameraSystem::move(float z, float dt)
{
    if (is_orbiter())
        m_orbiter.move(z);
    else 
        m_freefly.move(z, dt);
}

Point CameraSystem::position() const
{
    if (is_orbiter())
        return m_orbiter.position();
    else
        return m_freefly.position();
}

CameraType CameraSystem::type() const
{
    return m_type;
}

Camera &CameraSystem::freefly()
{
    return m_freefly;
}

Orbiter &CameraSystem::orbiter()
{
    return m_orbiter;
}
