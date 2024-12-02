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
    int init_shaders();
    int init_demo_scalar_field();

    int render_ui();
    int render_demo_buttons();
    int render_any();

    int update_height_field();

    int render_demo_scalar_field();
    int render_scalar_field_params();
    int render_scalar_field_stats();

    int handle_event();

    int render_menu_bar();

private:
    Mesh m_height_map;

    Framebuffer m_ImGUIFramebuffer;

    bool m_show_faces{true};
    bool m_show_edges{false};
    bool m_show_points{false};

    float m_size_point{5.f};
    float m_size_edge{2.f};

    float m_clear_color[3]{0.678f, 0.686f, 0.878f};
    float m_color_point[4]{0.0f, 0.0f, 1.0f, 1.0f};
    float m_color_edge[4]{1.0f, 1.0f, 0.0f, 1.0f};

    bool m_show_style_editor{false};
    bool m_show_ui{true};
    bool m_dark_theme{true};
    bool m_show_skybox{true};

    bool m_need_update{false};

    Timer m_timer;

    //! Demo boolean attributes
    bool m_activate_scalar_field_demo{true};
    bool m_activate_height_map_demo{false};

    //! Scalar field attributes
    Ref<mmv::HF> m_hf;

    std::vector<float> m_elevations{};

    float m_scale{50.0f};

    vec2 m_hf_a, m_hf_b;

    Point pmin, pmax;

    int m_hf_dim[2]{128, 128};
    int m_output_dim[2]{256, 256};

    int m_ww, m_wh; // window width/height

    //! Noise
    int m_noctaves{4};
    float m_amplitude{0.035f};
    float m_frequency{0.05f};
    int m_interpolation_func{1};

    float m_hurst{0.2f};
    float m_lacunarity{2.5f};
    float m_base_scale{0.005f};

    enum VAO_TYPE
    {
        OBJECT = 0,
        CUBEMAP,
        NB_VAO
    };

    GLuint m_vao[VAO_TYPE::NB_VAO];

    enum VBO_TYPE
    {
        POSITION = 0,
        TEXCOORD,
        NORMAL,
        COLOR,
        MATERIAL,
        TRANSFORM,
        NB_VBO
    };

    GLuint m_buffers[VBO_TYPE::NB_VBO];

    //! Shaders
    GLuint m_program_texture;
    GLuint m_program_skybox;
    GLuint m_program_points;
    GLuint m_program_edges;
    GLuint m_program_faces;

    //! Textures
    GLuint m_tex_skybox;
    GLuint m_tex_normal;
    GLuint m_tex_gradient;
    GLuint m_tex_elevation;
    GLuint m_tex_slope;

    enum OVERLAY_TEX
    {
        NONE_TEX=0,
        ELEVATION_TEX, 
        NORMAL_TEX, 
        GRADIENT_TEX, 
        SLOPE_TEX, 
        NB_TEX
    };

    OVERLAY_TEX m_overlay{OVERLAY_TEX::NONE_TEX}; 

    std::vector<float> m_positions;
    std::vector<float> m_texcoords;
    std::vector<float> m_normals;
};
