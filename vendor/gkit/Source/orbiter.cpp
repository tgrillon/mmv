
#include <cstdio>
#include <algorithm>

#include "orbiter.h"

void Orbiter::lookat( const Point& center, const float size )
{
    m_center= center;
    m_position= vec2(0, 0);
    m_rotation= vec2(0, 180);
    m_size= size;
    m_radius= size;
}

void Orbiter::lookat( const Point& pmin, const Point& pmax )
{
    lookat(center(pmin, pmax), distance(pmin, pmax));
}

void Orbiter::rotation( const float x, const float y )
{
    m_rotation.x= m_rotation.x + y;
    m_rotation.y= m_rotation.y + x;
}

void Orbiter::translation( const float x, const float y )
{
    m_position.x= m_position.x - m_size * x;
    m_position.y= m_position.y + m_size * y;
}

void Orbiter::move( const float z )
{
    m_size= m_size - m_size * 0.01f * z;
    if(m_size < 0.001f)
        m_size= 0.001f;
}

Transform Orbiter::view( ) const
{
    return Translation( -m_position.x, -m_position.y, -m_size ) 
        * RotationX(m_rotation.x) * RotationY(m_rotation.y) 
        * Translation( -m_center.x, -m_center.y, -m_center.z ); 
}

Transform Orbiter::projection( const int width, const int height, const float fov )
{
    m_width= width;
    m_height= height;
    m_fov= fov;
    
    return projection();
}

float Orbiter::znear( ) const
{
    // calcule la distance entre le centre de l'objet et la camera
    float d= distance(m_center, Point(m_position.x, m_position.y, m_size));
    return std::max(float(0.1), d - m_radius);
}

float Orbiter::zfar( ) const
{
    // calcule la distance entre le centre de l'objet et la camera
    float d= distance(m_center, Point(m_position.x, m_position.y, m_size));
    return std::max(float(1), d + m_radius);
}


Transform Orbiter::projection( ) const
{
    // calcule la distance entre le centre de l'objet et la camera
    //~ Transform t= view();
    //~ Point c= t(m_center);
    //~ float d= -c.z;
    //~ float d= distance(m_center, Point(m_position.x, m_position.y, m_size));     // meme resultat plus rapide a calculer
    
    // regle near et far en fonction du centre et du rayon englobant l'objet 
    return Perspective(m_fov, m_width / m_height, znear(), zfar());
}

Transform Orbiter::viewport( ) const
{
    return Viewport(m_width, m_height);
}

//~ void Orbiter::frame( const float width, const float height, const float z, const float fov, Point& dO, Vector& dx, Vector& dy ) const
void Orbiter::frame( const float z, Point& dO, Vector& dx, Vector& dy ) const
{
    Transform v= view();
    Transform p= projection();
    Transform viewport= Viewport(m_width, m_height);
    Transform t= viewport * p * v;              // passage monde vers image
    Transform tinv= t.inverse();                // l'inverse, passage image vers monde
    
    // origine du plan image
    dO= tinv(Point(0, 0, z));
    // axe x du plan image
    Point d1= tinv(Point(1, 0, z));
    // axe y du plan image
    Point d2= tinv(Point(0, 1, z));
    
    dx= Vector(dO, d1);
    dy= Vector(dO, d2);
}

Point Orbiter::position( ) const
{
    Transform t= view();     // passage monde vers camera
    Transform tinv= t.inverse();            // l'inverse, passage camera vers monde
    
    return tinv(Point(0, 0, 0));        // la camera se trouve a l'origine, dans le repere camera...
}

int Orbiter::read_orbiter( const char *filename )
{
    FILE *in= fopen(filename, "rt");
    if(in == NULL)
    {
        printf("[error] loading orbiter '%s'...\n", filename);
        return -1;
    }
    
    printf("loading orbiter '%s'...\n", filename);
    
    bool errors= false;
    if(fscanf(in, "c %f %f %f \n", &m_center.x, &m_center.y, &m_center.z) != 3)
        errors= true;
    if(fscanf(in, "p %f %f\n", &m_position.x, &m_position.y) != 2)
        errors= true;
    if(fscanf(in, "r %f %f\n", &m_rotation.x, &m_rotation.y) != 2)
        errors= true;
    if(fscanf(in, "s %f %f\n", &m_size, &m_radius) != 2)
        errors= true;
        
    if(fscanf(in, "f %f %f %f\n", &m_fov, &m_width, &m_height) != 3)
        errors= true;
    
    fclose(in);
    if(errors)
    {
        printf("[error] loading orbiter '%s'...\n", filename);
        return -1;
    }
    
    return 0;
}

int Orbiter::write_orbiter( const char *filename )
{
    FILE *out= fopen(filename, "wt");
    if(out == NULL)
        return -1;
    
    printf("writing orbiter '%s'...\n", filename);
    
    fprintf(out, "c %f %f %f\n", m_center.x, m_center.y, m_center.z);
    fprintf(out, "p %f %f\n", m_position.x, m_position.y);
    fprintf(out, "r %f %f\n", m_rotation.x, m_rotation.y);
    fprintf(out, "s %f %f\n", m_size, m_radius);
    fprintf(out, "f %f %f %f\n", m_fov, m_width, m_height);
    
    fclose(out);
    return 0;
}
