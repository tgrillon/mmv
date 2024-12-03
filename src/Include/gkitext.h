#pragma once

#include "pch.h"

Mesh make_grid(const int n = 10);

Mesh make_plane();

Mesh make_frustum();

Mesh make_xyz();

Point point_min(const Point &p1, const Point &p2);

Point point_max(const Point &p1, const Point &p2);

Mesh read_mesh(const std::string &obj);

GLuint read_program(const std::string &shader);

GLuint read_texture(const int unit, const std::string &texture);

Image read_image(const std::string &image);

GLuint read_cubemap(const int unit, const std::string &filename, const GLenum texel_type = GL_RGBA);
