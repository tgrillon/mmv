#include "Viewer.h"

#include "Utils.h"
#include "Buffer.h"
#include "gkitext.h"

Viewer::Viewer() : App(1024, 640), m_ImGUIFramebuffer(window_width(), window_height()), m_ww(window_width()), m_wh(window_height())
{
}

int Viewer::init_any()
{
    init_shaders();

    init_demo_scalar_field();

    glGenVertexArrays(VAO_TYPE::NB_VAO, m_vao);

    m_tex_skybox = read_cubemap(0, std::string(DATA_DIR) + "/skybox3.png", GL_RGBA);

    return 0;
}

int Viewer::init_imgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    return 0;
}

int Viewer::init_shaders()
{
    m_program_points = read_program(std::string(SHADER_DIR) + "/points.glsl");
    program_print_errors(m_program_points);

    m_program_edges = read_program(std::string(SHADER_DIR) + "/edges.glsl");
    program_print_errors(m_program_edges);

    m_program_skybox = read_program(std::string(SHADER_DIR) + "/cubemap.glsl");
    program_print_errors(m_program_skybox);

    m_program_faces = read_program(std::string(SHADER_DIR) + "/faces.glsl");
    program_print_errors(m_program_faces);

    m_program_texture = read_program(std::string(SHADER_DIR) + "/base_texture.glsl");
    program_print_errors(m_program_texture);
    return 0;
}

int Viewer::init_demo_scalar_field()
{
    //! Generate height map using Perlin noise

    m_elevations = znoise::generate_hmf("elevation.png", m_scale, m_hf_dim, m_hf_dim, m_hurst, m_lacunarity, m_base_scale);
    m_hf_a = {0.f, 0.f};
    m_hf_b = {(float)m_hf_dim, (float)m_hf_dim};

    m_hf = mmv::HF::Create(m_elevations, m_hf_a, m_hf_b, m_hf_dim, m_hf_dim);
    m_height_map = m_hf->Polygonize(m_resolution);

    m_height_map.bounds(pmin, pmax);
    m_cs.orbiter().lookat(pmin, pmax);

    m_hf->SaveGradientAsImage("gradient.png", m_output_dim, m_output_dim);
    m_hf->SaveNormalAsImage("normal.png", m_output_dim, m_output_dim);
    m_hf->SaveSlopeAsImage("slope.png", m_output_dim, m_output_dim);

    m_tex_elevation = read_texture(0, std::string(DATA_DIR) + "/output/elevation.png");
    m_tex_normal = read_texture(0, std::string(DATA_DIR) + "/output/normal.png");
    m_tex_gradient = read_texture(0, std::string(DATA_DIR) + "/output/gradient.png");
    m_tex_slope = read_texture(0, std::string(DATA_DIR) + "/output/slope.png");

    return 0;
}

int Viewer::render()
{
    if (render_ui() < 0)
    {
        utils::error("Error with the UI rendering!");
        return -1;
    }

    m_ImGUIFramebuffer.bind();
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], 1.f);

    glClearDepth(1.f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (render_any() < 0)
    {
        utils::error("Error with the geometry rendering!");
        return -1;
    }

    m_ImGUIFramebuffer.unbind();

    return 1;
}

int Viewer::quit_any()
{
    m_height_map.release();

    release_program(m_program_edges);
    release_program(m_program_points);
    release_program(m_program_faces);
    release_program(m_program_texture);
    release_program(m_program_skybox);

    glDeleteTextures(1, &m_tex_skybox);
    glDeleteTextures(1, &m_tex_normal);
    glDeleteTextures(1, &m_tex_gradient);
    glDeleteTextures(1, &m_tex_elevation);
    glDeleteTextures(1, &m_tex_slope);

    glDeleteVertexArrays(VAO_TYPE::NB_VAO, m_vao);
    glDeleteBuffers(VBO_TYPE::NB_VBO, m_buffers);

    return 0;
}

int Viewer::quit_imgui()
{
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

int Viewer::render_any()
{
    handle_event();

    Transform model = Identity();
    Transform view = m_cs.view();
    Transform projection = m_cs.projection();
    Transform viewport = m_cs.viewport();

    Transform mv = view * model;
    Transform mvp = projection * mv;

    Transform normalMatrix = mv.normal();
    Point light = m_cs.position();

    DrawParam param;
    param.model(model).view(view).projection(projection);

    if (m_show_faces)
    {
        if (m_overlay != OVERLAY_TEX::NONE_TEX)
        {
            glUseProgram(m_program_texture);
            program_uniform(m_program_texture, "u_MvpMatrix", mvp);
            program_uniform(m_program_texture, "u_MvMatrix", mv);
            program_uniform(m_program_texture, "u_NormalMatrix", normalMatrix);
            program_uniform(m_program_texture, "u_Light", view(light));

            switch (m_overlay)
            {
            case OVERLAY_TEX::ELEVATION_TEX:
                program_use_texture(m_program_texture, "u_Texture", 0, m_tex_elevation);
                break;
            case OVERLAY_TEX::NORMAL_TEX:
                program_use_texture(m_program_texture, "u_Texture", 0, m_tex_normal);
                break;
            case OVERLAY_TEX::GRADIENT_TEX:
                program_use_texture(m_program_texture, "u_Texture", 0, m_tex_gradient);
                break;
            case OVERLAY_TEX::SLOPE_TEX:
                program_use_texture(m_program_texture, "u_Texture", 0, m_tex_slope);
                break;
            }

            m_height_map.draw(m_program_texture, true, true, true, false, false);
        }
        else
        {
            glUseProgram(m_program_faces);
            program_uniform(m_program_faces, "u_MvpMatrix", mvp);
            program_uniform(m_program_faces, "u_MvMatrix", mv);
            program_uniform(m_program_faces, "u_NormalMatrix", normalMatrix);
            program_uniform(m_program_faces, "u_Light", view(light));
            m_height_map.draw(m_program_faces, true, false, true, false, false);
        }
    }

    if (m_show_edges)
    {
        glUseProgram(m_program_edges);

        glLineWidth(m_size_edge);
        program_uniform(m_program_edges, "u_MvpMatrix", mvp);
        GLint location = glGetUniformLocation(m_program_edges, "u_EdgeColor");
        glUniform4fv(location, 1, &m_color_edge[0]);

        m_height_map.draw(m_program_edges, true, false, false, false, false);
    }

    if (m_show_points)
    {
        glUseProgram(m_program_points);

        program_uniform(m_program_points, "u_MvpMatrix", mvp);
        program_uniform(m_program_points, "u_PointSize", m_size_point);
        GLint location = glGetUniformLocation(m_program_points, "u_PointColor");
        glUniform4fv(location, 1, &m_color_point[0]);

        glDrawArrays(GL_POINTS, 0, m_height_map.vertex_count());
    }

    //! Render skybox
    if (m_show_skybox)
    {
        Transform inv = Inverse(viewport * projection * view);

        glUseProgram(m_program_skybox);
        glBindVertexArray(m_vao[VAO_TYPE::CUBEMAP]);
        program_uniform(m_program_skybox, "u_InvMatrix", inv);
        program_uniform(m_program_skybox, "u_CameraPosition", m_cs.position());

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_skybox);
        program_uniform(m_program_skybox, "u_Skybox", int(0));

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glUseProgram(0);
        glBindVertexArray(0);
    }

    return 0;
}

int Viewer::update_height_field()
{
    m_elevations = znoise::generate_hmf("elevation.png", m_scale, m_hf_dim, m_hf_dim, m_hurst, m_lacunarity, m_base_scale);

    m_hf->Elevations(m_elevations, m_hf_dim, m_hf_dim);
    m_height_map = m_hf->Polygonize(m_resolution);
    m_hf->SaveGradientAsImage("gradient.png", m_output_dim, m_output_dim);
    m_hf->SaveNormalAsImage("normal.png", m_output_dim, m_output_dim);
    m_hf->SaveSlopeAsImage("slope.png", m_output_dim, m_output_dim);
    m_tex_elevation = read_texture(0, std::string(DATA_DIR) + "/output/elevation.png");
    m_tex_normal = read_texture(0, std::string(DATA_DIR) + "/output/normal.png");
    m_tex_gradient = read_texture(0, std::string(DATA_DIR) + "/output/gradient.png");
    m_tex_slope = read_texture(0, std::string(DATA_DIR) + "/output/slope.png");

    return 0;
}

int Viewer::render_scalar_field_params()
{
    if (ImGui::SliderFloat("Scale", &m_scale, 1.0f, 64.f))
    {
        update_height_field();
    }
    ImGui::SliderInt("Scalar Field Dim", &m_hf_dim, 16.f, 1024.f);
    ImGui::SliderInt("Output Dim", &m_output_dim, 16.f, 1024.f);
    ImGui::SliderInt("Resolution", &m_resolution, (float)m_hf_dim, (float)m_hf_dim * 8);
    if (ImGui::CollapsingHeader("Perlin Noise"))
    {

        ImGui::SliderFloat("Hurst", &m_hurst, 0.01f, 5.0f);
        ImGui::SliderFloat("Lacunarity", &m_lacunarity, 1.f, 15.f);
        ImGui::SliderFloat("Base Scale", &m_base_scale, 0.001f, 1.f);
    }

    if (ImGui::Button("Generate"))
    {
        update_height_field();
    }

    if (ImGui::Button("Center camera"))
    {
        center_camera(m_height_map);
    }
    return 0;
}

int Viewer::handle_event()
{
    if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
    {
        if (key_state(SDLK_k))
        {
            clear_key_state(SDLK_k);
            m_show_skybox = !m_show_skybox;
        }
        if (key_state(SDLK_TAB))
        {
            clear_key_state(SDLK_TAB);
            m_show_ui = !m_show_ui;
        }
        if (key_state(SDLK_f))
        {
            clear_key_state(SDLK_f);
            m_show_faces = !m_show_faces;
        }
        if (key_state(SDLK_e))
        {
            clear_key_state(SDLK_e);
            m_show_edges = !m_show_edges;
        }
        if (key_state(SDLK_v))
        {
            clear_key_state(SDLK_v);
            m_show_points = !m_show_points;
        }

        float dt = delta_time() / 1000.f;
        if (key_state(SDLK_z))
        {
            m_cs.freefly().translation(CameraMovement::FORWARD, dt);
        }

        if (key_state(SDLK_q))
        {
            m_cs.freefly().translation(CameraMovement::LEFT, dt);
        }

        if (key_state(SDLK_s))
        {
            m_cs.freefly().translation(CameraMovement::BACKWARD, dt);
        }

        if (key_state(SDLK_d))
        {
            m_cs.freefly().translation(CameraMovement::RIGHT, dt);
        }

        if (key_state(SDLK_SPACE))
        {
            m_cs.freefly().translation(CameraMovement::UP, dt);
        }

        if (key_state(SDLK_LSHIFT))
        {
            m_cs.freefly().translation(CameraMovement::DOWN, dt);
        }

        if (key_state(SDLK_x))
        {
            clear_key_state(SDLK_x);
            m_cs.toggle_type();
            if (m_cs.type() == CameraType::ORBITER)
            {
                m_cs.orbiter().lookat(pmin, pmax);
            }
        }
    }

    return 0;
}

int Viewer::render_ui()
{
    ImGui::DockSpaceOverViewport();

    if (render_menu_bar() < 0)
    {
        utils::error("Error with the menu bar rendering!");
        return -1;
    }

    ImGui::Begin("Viewport");

    if (ImGui::IsWindowFocused())
    {
        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        io.WantCaptureMouse = false;
        io.WantCaptureKeyboard = false;
    }

    // we access the ImGui window size
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    if (window_width > 0 && window_height > 0 && ((float)m_ww != window_width || (float)m_wh != window_height))
    {
        // we rescale the framebuffer to the actual window size here and reset the glViewport
        m_ImGUIFramebuffer.rescale(window_width, window_height);
        glViewport(0, 0, window_width, window_height);
        m_cs.projection(window_width, window_height);
    }

    // we get the screen position of the window
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        (ImTextureID)(intptr_t)m_ImGUIFramebuffer.texture_id(),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + window_width, pos.y + window_height),
        ImVec2(0, 1),
        ImVec2(1, 0));

    ImGui::End();

    if (m_show_ui)
    {
        ImGui::Begin("Map");
        ImGui::Image((ImTextureID)(intptr_t)m_tex_elevation, ImVec2(128, 128), {0, 1}, {1, 0});
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip("Elevation");
        }
        if (ImGui::IsItemClicked())
        {
            m_overlay = OVERLAY_TEX::ELEVATION_TEX;
        }
        ImGui::SameLine();
        ImGui::Image((ImTextureID)(intptr_t)m_tex_gradient, ImVec2(128, 128), {0, 1}, {1, 0});
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip("Gradient");
        }
        if (ImGui::IsItemClicked())
        {
            m_overlay = OVERLAY_TEX::GRADIENT_TEX;
        }
        ImGui::SameLine();
        ImGui::Image((ImTextureID)(intptr_t)m_tex_normal, ImVec2(128, 128), {0, 1}, {1, 0});
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip("Normal");
        }
        if (ImGui::IsItemClicked())
        {
            m_overlay = OVERLAY_TEX::NORMAL_TEX;
        }
        ImGui::SameLine();
        ImGui::Image((ImTextureID)(intptr_t)m_tex_slope, ImVec2(128, 128), {0, 1}, {1, 0});
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip("Slope");
        }
        if (ImGui::IsItemClicked())
        {
            m_overlay = OVERLAY_TEX::SLOPE_TEX;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear overlay", ImVec2(128, 128)))
        {
            m_overlay = OVERLAY_TEX::NONE_TEX;
        }
        ImGui::End();

        ImGui::Begin("Control Panel");

        render_demo_buttons();

        ImGui::Checkbox("Show Skybox", &m_show_skybox);
        if (ImGui::CollapsingHeader("Camera"))
        {
            ImGui::SliderFloat("FOV", &m_cs.fov(), 35.0f, 120.0f);
            if (m_cs.is_freefly())
            {
                ImGui::SliderFloat("Movement speed", &m_cs.freefly().movement_speed(), 1.0f, 50.0f);
                ImGui::SliderFloat("Rotation speed", &m_cs.freefly().rotation_speed(), 0.1f, 1.0f);
                ImGui::InputFloat3("Position", &m_cs.freefly().position().x);
            }
        }

        if (ImGui::CollapsingHeader("Global"))
        {
            ImGui::SliderFloat("Point size", &m_size_point, 1.f, 50.f, "%.2f");
            ImGui::SliderFloat("Edge size", &m_size_edge, 1.f, 25.f, "%.2f");

            ImGui::Checkbox("Faces (f)", &m_show_faces);
            ImGui::SameLine();
            ImGui::Checkbox("Edges (e)", &m_show_edges);
            ImGui::SameLine();
            ImGui::Checkbox("Points (v)", &m_show_points);

            if (ImGui::CollapsingHeader("Colors"))
            {
                ImGui::ColorPicker3("Clear color", &m_clear_color[0]);
                ImGui::ColorPicker3("Point color", &m_color_point[0]);
                ImGui::ColorPicker3("Edge color", &m_color_edge[0]);
            }
        }
        if (ImGui::CollapsingHeader("Params"))
        {
            render_scalar_field_params();
        }
        ImGui::End();

        ImGui::Begin("Statistiques");

        ImGui::SeparatorText("Performances");
        auto [cpums, cpuus] = cpu_time();
        auto [gpums, gpuus] = gpu_time();
        ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
        ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
        ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
        ImGui::Text("frame rate : %.2f ms", delta_time());
        ImGui::SeparatorText("Geometry");
        ImGui::End();
    }

    if (m_show_style_editor)
    {
        ImGui::Begin("Dear ImGui Style Editor", &m_show_style_editor);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }

    ImGui::Render();

    return 0;
}

int Viewer::render_demo_buttons()
{
    ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);

    if (ImGui::CollapsingHeader("Menu"))
    {
        //! Scalar field demo widget
        ImGui::BeginDisabled(m_activate_scalar_field_demo);
        if (ImGui::Button("Scalar Field demo", sz) && !m_activate_scalar_field_demo)
        {
            m_activate_height_map_demo = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(!m_activate_scalar_field_demo ? "Activate scalar field demo." : "Scalar field demo is active.");
        }
        ImGui::EndDisabled();

        //! Height map demo widget
        ImGui::BeginDisabled(m_activate_height_map_demo);
        if (ImGui::Button("Height map Demo", sz) && !m_activate_height_map_demo)
        {
            m_activate_scalar_field_demo = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(!m_activate_height_map_demo ? "Activate height map demo." : "Height map demo is active.");
        }
        ImGui::EndDisabled();
    }

    return 0;
}

int Viewer::render_menu_bar()
{
    ImGui::DockSpace(ImGui::GetID("DockSpace"));

    if (ImGui::BeginMainMenuBar())
    {
        // ImGui::MenuItem("Style Editor", NULL, &m_show_style_editor);
        ImGui::MenuItem("Exit", NULL, &m_exit);
        ImGui::MenuItem(m_show_ui ? "Hide UI" : "Show UI", NULL, &m_show_ui);
        if (ImGui::MenuItem(m_dark_theme ? "Light Theme" : "Dark Theme", NULL, &m_dark_theme))
        {
            if (m_dark_theme)
            {
                ImGui::StyleColorsDark();
            }
            else
            {
                ImGui::StyleColorsLight();
            }
        }

        ImGui::EndMainMenuBar();
    }

    // ImGui::ShowDemoWindow();

    return 0;
}
