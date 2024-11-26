#pragma once

#include "pch.h"

#include "App.h"
#include "Framebuffer.h"
#include "Timer.h"
#include "HeightField.h"
#include "PerlinNoise.h"

class Viewer : public App
{
public:
    Viewer();

    int init_any() override;
    int init_imgui() override;

    int render() override;

    int quit_any() override;
    int quit_imgui() override;

private:
    int init_demo_scalar_field();

    int render_ui();
    int render_demo_buttons();
    int render_any();

    int render_demo_scalar_field();
    int render_scalar_field_params();
    int render_scalar_field_stats();

    int handle_event();

    int render_menu_bar();

private:
    Mesh m_grid; 

    GLuint m_program_points;
    GLuint m_program_edges;

    Framebuffer m_framebuffer;

    bool m_show_faces{true};
    bool m_show_edges{false};
    bool m_show_points{false};

    float m_size_point{5.f};
    float m_size_edge{2.f};

    float m_color_point[4]{0.0f, 0.0f, 1.0f, 1.0f};
    float m_color_edge[4]{1.0f, 1.0f, 0.0f, 1.0f};

    bool m_show_style_editor{false};
    bool m_show_ui{true};
    bool m_dark_theme{true};

    bool m_need_update{false};

    Timer m_timer;

    //! Demo boolean attributes
    bool m_activate_scalar_field_demo {true}; 
    bool m_activate_height_map_demo {false}; 

    //! Scalar field attributes
    Ref<mmv::SF> m_sf;

    vec2 m_sf_a, m_sf_b;

    Point pmin, pmax;

    int m_sf_dim[2] {128, 128};
    int m_output_dim[2] {256, 256};

    int m_noctaves{4};
    float m_amplitude{0.035f};
    float m_frequency{0.05f};

    int m_interpolation_func{1};

    GLuint m_vao;
    
    enum VBO_TYPE
    {
        POSITION = 0,
        TEXCOORD,
        NORMAL,
        COLOR,
        MATERIAL,
        TRANSFORM,
        NB_ELT
    };

    GLuint m_buffers[NB_ELT];

    GLuint m_program; 
    GLuint m_texture_noise;

    std::vector<float> m_positions;
    std::vector<float> m_texcoords;
    std::vector<float> m_normals;
};
