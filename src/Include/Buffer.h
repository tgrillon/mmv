#pragma once

#include "pch.h"

void load_buffer(int vbo, int location, int size, const std::vector<float> &data, GLenum mode = GL_STATIC_DRAW);

void load_buffer(int vbo, int location, int size, const std::vector<int> &data, GLenum mode = GL_STATIC_DRAW);

// Load a VBO for an instance vertex attribute
void load_buffer_instance(int vbo, int location, int size, const std::vector<float> &data, GLenum mode = GL_STATIC_DRAW);
void load_buffer_instance(int vbo, int location, int size, const std::vector<int> &data, GLenum mode = GL_STATIC_DRAW);

void update_buffer(int vbo, const std::vector<float> &data);
void update_buffer(int vbo, const std::vector<int> &data);
