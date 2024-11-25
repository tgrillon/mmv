#include "Viewer.h"

#include "Utils.h"

Mesh make_grid(const int n = 10)
{
    Mesh grid = Mesh(GL_LINES);

    // grille
    grid.color(White());
    for (int x = 0; x < n; x++)
    {
        float px = float(x) - float(n) / 2 + .5f;
        grid.vertex(Point(px, 0, -float(n) / 2 + .5f));
        grid.vertex(Point(px, 0, float(n) / 2 - .5f));
    }

    for (int z = 0; z < n; z++)
    {
        float pz = float(z) - float(n) / 2 + .5f;
        grid.vertex(Point(-float(n) / 2 + .5f, 0, pz));
        grid.vertex(Point(float(n) / 2 - .5f, 0, pz));
    }

    // axes XYZ
    grid.color(Red());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(1, .1, 0));

    grid.color(Green());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(0, 1, 0));

    grid.color(Blue());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(0, .1, 1));

    glLineWidth(2);

    return grid;
}

GLuint read_program(const std::string &filepath)
{
    return read_program(filepath.c_str());
}

Viewer::Viewer() : App(1024, 640), m_framebuffer(window_width(), window_height())
{
}

int Viewer::init_any()
{
    m_program_points = read_program(std::string(SHADER_DIR) + "/points.glsl");
    program_print_errors(m_program_points);

    m_program_edges = read_program(std::string(SHADER_DIR) + "/edges.glsl");
    program_print_errors(m_program_edges);

    m_grid = make_grid();

    m_grid.bounds(pmin, pmax);
    m_cs.orbiter().lookat(pmin, pmax);

    init_demo_scalar_field();

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

int Viewer::render()
{
    if (render_ui() < 0)
    {
        utils::error("Error with the UI rendering!");
        return -1;
    }

    m_framebuffer.bind();
    glClearColor(0.678f, 0.686f, 0.878f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glEnable(GL_DEPTH_TEST);

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

    delete_exprtk(m_expr_sf);

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

    Transform mvp = projection * view * model;

    DrawParam param;
    param.model(model).view(view).projection(projection);

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

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

    return 0;
}

int Viewer::handle_event()
{
    utils::info("Camera : ", (m_cs.is_orbiter() ? "ORBITER" : "FREEFLY"));
    if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
    {
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

int Viewer::init_demo_scalar_field()
{
    m_expr_sf = exprtk_wrapper_init();
    add_double_variable(m_expr_sf, "x");
    add_double_variable(m_expr_sf, "y");
    set_expression_count(m_expr_sf, 2);

    // std::vector<float> heights;
    // mmv::surface_points(m_resolution, [&](double x_val, double y_val)
    //                     {
    //                 set_double_variable_value(m_expr_sf, "x", x_val);
    //                 set_double_variable_value(m_expr_sf, "y", y_val);
    //                 return get_evaluated_value(m_expr_sf, 0); });

    // m_sf = mmv::SF::Create(heights, m_sf_a, m_sf_b, m_sf_nx, m_sf_ny);

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
        (void*)m_framebuffer.texture_id(),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + window_width, pos.y + window_height),
        ImVec2(0, 1),
        ImVec2(1, 0));

    ImGui::End();

    if (m_show_ui)
    {
        ImGui::Begin("Control Panel");

        render_demo_buttons();

        if (m_cs.is_freefly())
        {
            utils::print("Here!");
            if (ImGui::CollapsingHeader("Camera"))
            {
                ImGui::SliderFloat("Movement speed", &m_cs.freefly().movement_speed(), 1.0f, 50.0f);
                ImGui::SliderFloat("Rotation speed", &m_cs.freefly().rotation_speed(), 0.1f, 1.0f);
                ImGui::SliderFloat("FOV", &m_cs.freefly().fov(), 35.0f, 120.0f);
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
                ImGui::ColorPicker3("Point color", &m_color_point[0]);
                ImGui::ColorPicker3("Edge color", &m_color_edge[0]);
            }
        }
        if (ImGui::CollapsingHeader("Params"))
        {
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
