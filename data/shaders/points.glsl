#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 aPosition;

uniform mat4 u_MvpMatrix;
uniform float u_PointSize;

void main()
{   
    gl_Position= u_MvpMatrix * vec4(aPosition, 1.0); 
    gl_PointSize= u_PointSize;
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 out_color; 

uniform vec4 u_PointColor=vec4(1., 0., 0., 1.); 

void main()
{
    out_color= u_PointColor;
}
#endif
