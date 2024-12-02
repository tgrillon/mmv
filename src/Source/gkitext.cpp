#include "gkitext.h"

Mesh make_grid(const int n)
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

Point point_min(const Point &p1, const Point &p2)
{
    return {std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)};
}

Point point_max(const Point &p1, const Point &p2)
{
    return {std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z)};
}

Mesh read_mesh(const std::string &obj)
{
    return read_mesh(obj.c_str());
}

GLuint read_program(const std::string &shader)
{
    return read_program(shader.c_str());
}

GLuint read_texture(const std::string &texture)
{
    return read_texture(0, texture.c_str());
}

Image read_image(const std::string &image)
{
    return read_image(image.c_str());
}

Mesh make_plane()
{
    Mesh plane = Mesh(GL_TRIANGLE_STRIP);

    // Couleur de la grille
    plane.color(Color(0.6, 0.1, 0.5));

    plane.normal(0, 1, 0);
    for (int i = -5; i <= 5; ++i)
        for (int j = -5; j <= 5; ++j)
        {
            plane.vertex(-5, 0, j);
            plane.vertex(5, 0, j);

            plane.vertex(i, 0, -5);
            plane.vertex(i, 0, 5);
        }

    return plane;
}

Mesh make_frustum()
{
    glLineWidth(2);
    Mesh frustum = Mesh(GL_LINES);

    frustum.color(Yellow());
    // face avant
    frustum.vertex(-1, -1, -1);
    frustum.vertex(-1, 1, -1);
    frustum.vertex(-1, 1, -1);
    frustum.vertex(1, 1, -1);
    frustum.vertex(1, 1, -1);
    frustum.vertex(1, -1, -1);
    frustum.vertex(1, -1, -1);
    frustum.vertex(-1, -1, -1);

    // face arriere
    frustum.vertex(-1, -1, 1);
    frustum.vertex(-1, 1, 1);
    frustum.vertex(-1, 1, 1);
    frustum.vertex(1, 1, 1);
    frustum.vertex(1, 1, 1);
    frustum.vertex(1, -1, 1);
    frustum.vertex(1, -1, 1);
    frustum.vertex(-1, -1, 1);

    // aretes
    frustum.vertex(-1, -1, -1);
    frustum.vertex(-1, -1, 1);
    frustum.vertex(-1, 1, -1);
    frustum.vertex(-1, 1, 1);
    frustum.vertex(1, 1, -1);
    frustum.vertex(1, 1, 1);
    frustum.vertex(1, -1, -1);
    frustum.vertex(1, -1, 1);

    return frustum;
}

Mesh make_xyz()
{
    glLineWidth(2);
    Mesh axis = Mesh(GL_LINES);

    // axes XYZ
    axis.color(Red());
    axis.vertex(Point(0, 0, 0));
    axis.vertex(Point(1, 0, 0));

    axis.color(Green());
    axis.vertex(Point(0, 0, 0));
    axis.vertex(Point(0, 1, 0));

    axis.color(Blue());
    axis.vertex(Point(0, 0, 0));
    axis.vertex(Point(0, 0, 1));

    return axis;
}

//! charge une image, decoupe les 6 faces et renvoie une texture cubemap.
GLuint read_cubemap(const int unit, const std::string &filename, const GLenum texel_type)
{
    // les 6 faces sur une croix
    ImageData image = read_image_data(filename.c_str());
    if (image.pixels.empty())
        return 0;

    int w = image.width / 4;
    int h = image.height / 3;
    assert(w == h);

    GLenum data_format;
    GLenum data_type = GL_UNSIGNED_BYTE;
    if (image.channels == 3)
        data_format = GL_RGB;
    else // par defaut
        data_format = GL_RGBA;

    // creer la texture
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    // creer les 6 faces
    // chaque face de la cubemap est un rectangle [image.width/4 x image.height/3] dans l'image originale
    struct
    {
        int x, y;
    } faces[] = {
        {0, 1}, // X+
        {2, 1}, // X-
        {1, 2}, // Y+
        {1, 0}, // Y-
        {1, 1}, // Z+
        {3, 1}, // Z-
    };

    for (int i = 0; i < 6; i++)
    {
        ImageData face = flipX(flipY(copy(image, faces[i].x * w, faces[i].y * h, w, h)));

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     texel_type, w, h, 0,
                     data_format, data_type, face.data());
    }

    // parametres de filtrage
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // filtrage "correct" sur les bords du cube...
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    //~ glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    printf("  cubemap faces %dx%d\n", w, h);

    return texture;
}
