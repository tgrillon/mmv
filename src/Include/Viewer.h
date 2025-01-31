#pragma once

#include "pch.h"

#include "App.h"
#include "Framebuffer.h"
#include "Timer.h"
#include "HeightField.h"

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

    int update_height_field(bool export_elevation=true);

    int render_demo_scalar_field();
    int render_scalar_field_params();
    int render_scalar_field_stats();

    int save_params();
    int load_params();

    int handle_event();

    int render_menu_bar();

private:
    Mesh m_height_map;

    //! Application params
    Framebuffer m_ImGUIFramebuffer;
    
    Timer m_timer;

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

    Point pmin, pmax;

    //! Demo boolean attributes
    bool m_activate_scalar_field_demo{true};
    bool m_activate_height_map_demo{false};

    //! Scalar field attributes
    Ref<mmv::HF> m_hf;

    std::vector<float> m_elevations{};

    float m_scale{50.0f};

    vec2 m_hf_a, m_hf_b;

    int m_hf_dim{128};
    int m_output_dim{256};

    int m_framebuffer_width, m_framebuffer_height; // window width/height

    int m_resolution{128};

    int m_map_dim{128};

    //! Noise
    float m_hurst{0.2f};
    float m_lacunarity{2.5f};
    float m_base_scale{0.005f};
    int m_offset[2]{0, 0};
    int m_seed{0};

    //! VAO & VBO
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

    std::vector<float> m_positions;
    std::vector<float> m_texcoords;
    std::vector<float> m_normals;

    Vector m_object_scale{1.f, 1.f, 1.f};

    //! Shaders
    GLuint m_program_texture{0};
    GLuint m_program_skybox{0};
    GLuint m_program_points{0};
    GLuint m_program_edges{0};
    GLuint m_program_faces{0};

    //! Textures
    GLuint m_tex_skybox{0};
    GLuint m_tex_gradient{0};
    GLuint m_tex_elevation{0};
    GLuint m_tex_laplacian{0};
    GLuint m_tex_normal{0};
    GLuint m_tex_slope{0};
    GLuint m_tex_avg_slope{0};
    GLuint m_tex_shading{0};
    GLuint m_tex_stream_area{0};

    Vector m_shading_dir{-1.f, -1.f, -1.f};

    std::string m_filename{""};

    enum OVERLAY_TEX
    {
        NONE_TEX=0,
        ELEVATION_TEX, 
        GRADIENT_TEX, 
        LAPLACIAN_TEX,
        NORMAL_TEX, 
        SLOPE_TEX, 
        AVG_SLOPE_TEX, 
        SHADING_TEX, 
        STREAM_AREA_TEX, 
        NB_TEX
    };

    OVERLAY_TEX m_overlay{OVERLAY_TEX::NONE_TEX}; 
};
