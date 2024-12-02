#include "Viewer.h"

#include "Utils.h"
#include "Buffer.h"
#include "gkitext.h"

Viewer::Viewer() : App(1024, 640), m_framebuffer(window_width(), window_height())
{
}

int Viewer::init_any()
{
    m_program_points = read_program(std::string(SHADER_DIR) + "/points.glsl");
    program_print_errors(m_program_points);

    m_program_edges = read_program(std::string(SHADER_DIR) + "/edges.glsl");
    program_print_errors(m_program_edges);

    m_program_skybox = read_program(std::string(SHADER_DIR) + "/cubemap.glsl");

    m_grid = make_grid();

    m_grid.bounds(pmin, pmax);
    m_cs.orbiter().lookat(pmin, pmax);

    init_demo_scalar_field();

    m_texture_skybox = read_cubemap(0, std::string(DATA_DIR) + "/skybox2.jpg", GL_RGBA);

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

int Viewer::init_demo_scalar_field()
{
    //! Generate height map using Perlin noise
    m_sf_dim[0] = m_sf_dim[1] = 128;

    std::vector<float> elevations = mmv::generate_height_map(m_sf_dim[0], m_sf_dim[1], m_noctaves, m_amplitude, m_frequency, (unsigned char)m_interpolation_func);
    m_sf_a = {0.f, 0.f};
    m_sf_b = {(float)m_sf_dim[0], (float)m_sf_dim[1]};

    m_sf = mmv::SF::Create(elevations, m_sf_a, m_sf_b, m_sf_dim[0], m_sf_dim[1]);

    m_sf->SaveHeightAsImage("noise.png", m_output_dim[0], m_output_dim[1]);
    m_sf->SaveHeightAsTxt("noise.txt", m_output_dim[0], m_output_dim[1]);
    // m_sf->SaveGradientAsImage("gradient.png", m_output_dim[0], m_output_dim[1]);

    //! Initialize buffers to draw a quad on which will be displayed the generated noise texture
    m_positions = {
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        0.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        0.f, 1.f, 0.f};

    m_texcoords = {
        0.f, 0.f,
        1.f, 0.f,
        1.f, 1.f,
        0.f, 0.f,
        1.f, 1.f,
        0.f, 1.f};

    m_normals = {
        0.f, 0.f, -1.f,
        0.f, 0.f, -1.f,
        0.f, 0.f, -1.f,
        0.f, 0.f, -1.f,
        0.f, 0.f, -1.f,
        0.f, 0.f, -1.f};

    glGenVertexArrays(VAO_TYPE::NB_VAO, m_vao);
    glBindVertexArray(m_vao[VAO_TYPE::OBJECT]);

    glGenBuffers(VBO_TYPE::NB_VBO, m_buffers);

    load_buffer(m_buffers[VBO_TYPE::POSITION], VBO_TYPE::POSITION, 3, m_positions);
    load_buffer(m_buffers[VBO_TYPE::TEXCOORD], VBO_TYPE::TEXCOORD, 2, m_texcoords);
    load_buffer(m_buffers[VBO_TYPE::NORMAL], VBO_TYPE::NORMAL, 3, m_normals);

    m_texture_noise = read_texture(std::string(DATA_DIR) + "/output/noise.png");

    glBindTexture(GL_TEXTURE_2D, m_texture_noise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    m_program = read_program(std::string(DATA_DIR) + "/shaders/base_texture.glsl");

    m_cs.orbiter().lookat({0.f, 0.f, 0.f}, {1.f, 1.f, 0.f});

    return 0;
}

int Viewer::render()
{
    if (render_ui() < 0)
    {
        utils::error("Error with the UI rendering!");
        return -1;
    }

    m_framebuffer.bind();
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

    m_framebuffer.unbind();

    return 1;
}

int Viewer::quit_any()
{
    m_grid.release();

    release_program(m_program);
    release_program(m_program_skybox);
    release_program(m_program_edges);
    release_program(m_program_points);

    glDeleteTextures(1, &m_texture_noise);
    glDeleteTextures(1, &m_texture_skybox);
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

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    glUseProgram(m_program);
    program_uniform(m_program, "uMvpMatrix", mvp);
    program_uniform(m_program, "uMvMatrix", mv);
    program_uniform(m_program, "uNormalMatrix", normalMatrix);
    program_uniform(m_program, "uLight", view(light));

    program_use_texture(m_program, "uTexture", 0, m_texture_noise);

    assert(m_vao != 0);
    glBindVertexArray(m_vao[VAO_TYPE::OBJECT]);
    glDrawArrays(GL_TRIANGLES, 0, m_positions.size() / 3);

    glBindVertexArray(m_vao[VAO_TYPE::CUBEMAP]);
    if (m_show_skybox)
    {
        Transform inv = Inverse(viewport * projection * view);

        glUseProgram(m_program_skybox);
        glBindVertexArray(m_vao[VAO_TYPE::CUBEMAP]);
        program_uniform(m_program_skybox, "u_InvMatrix", inv);
        program_uniform(m_program_skybox, "u_CameraPosition", m_cs.position());

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture_skybox);
        program_uniform(m_program_skybox, "u_Skybox", int(0));

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glUseProgram(0);
        glBindVertexArray(0);
    }

    /*
    if (m_show_faces)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glDepthFunc(GL_LESS);
        param.draw(m_grid);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (m_show_edges)
    {
        glUseProgram(m_program_edges);

        glLineWidth(m_size_edge);
        program_uniform(m_program_edges, "uMvpMatrix", mvp);
        GLint location = glGetUniformLocation(m_program_edges, "uEdgeColor");
        glUniform4fv(location, 1, &m_color_edge[0]);

        m_grid.draw(m_program_edges, true, false, false, false, false);
    }

    if (m_show_points)
    {
        glUseProgram(m_program_points);

        program_uniform(m_program_points, "uMvpMatrix", mvp);
        program_uniform(m_program_points, "uPointSize", m_size_point);
        GLint location = glGetUniformLocation(m_program_points, "uPointColor");
        glUniform4fv(location, 1, &m_color_point[0]);

        glDrawArrays(GL_POINTS, 0, m_grid.vertex_count());
    }
    */

    return 0;
}

int Viewer::render_scalar_field_params()
{
    if (ImGui::CollapsingHeader("Perlin Noise"))
    {
        ImGui::InputInt2("Scalar Field Dim", &m_sf_dim[0]);
        ImGui::InputInt2("Output Dim", &m_output_dim[0]);

        ImGui::SliderInt("N Octaves", &m_noctaves, 1, 16);
        ImGui::InputFloat("Amplitude", &m_amplitude);
        ImGui::InputFloat("Frequency", &m_frequency);
        ImGui::SliderInt("I Func", &m_interpolation_func, 0, 2);

        if (ImGui::Button("Generate noise") || key_state(SDLK_g))
        {
            clear_key_state(SDLK_g);
            std::vector<float> elevations = mmv::generate_height_map(m_sf_dim[0], m_sf_dim[1], m_noctaves, m_amplitude, m_frequency, (unsigned char)m_interpolation_func);

            m_sf->Elevations(elevations, m_sf_dim[0], m_sf_dim[1]);

            m_sf->SaveHeightAsImage("noise.png", m_output_dim[0], m_output_dim[1]);
            m_sf->SaveHeightAsTxt("noise.txt", m_output_dim[0], m_output_dim[1]);
            // m_sf->SaveGradientAsImage("gradient.png", m_output_dim[0], m_output_dim[1]);

            m_texture_noise = read_texture(std::string(DATA_DIR) + "/output/noise.png");
        }
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

    // we rescale the framebuffer to the actual window size here and reset the glViewport
    m_framebuffer.rescale(window_width, window_height);
    glViewport(0, 0, window_width, window_height);
    m_cs.projection(window_width, window_height);

    // we get the screen position of the window
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        (void *)m_framebuffer.texture_id(),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + window_width, pos.y + window_height),
        ImVec2(0, 1),
        ImVec2(1, 0));

    ImGui::End();

    if (m_show_ui)
    {
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

        if (ImGui::CollapsingHeader("Performances"))
        {
            auto [cpums, cpuus] = cpu_time();
            auto [gpums, gpuus] = gpu_time();
            ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
            ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
            ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
            ImGui::Text("frame rate : %.2f ms", delta_time());
        }
        if (ImGui::CollapsingHeader("Geometry"))
        {
        }
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
