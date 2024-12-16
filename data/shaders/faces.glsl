#version 330

#ifdef VERTEX_SHADER
layout(location=0) in vec3 a_Position;
layout(location=2) in vec3 a_Normal;

uniform mat4 u_MvpMatrix; 
uniform mat4 u_MvMatrix; 
uniform mat4 u_NormalMatrix; 

out vec3 vPosition; 
out vec3 vNormal; 

void main(void)
{
    vNormal = mat3(u_NormalMatrix) * a_Normal; 
    vPosition = vec3(u_MvMatrix * vec4(a_Position, 1.0));
    gl_Position = u_MvpMatrix * vec4(a_Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER
in vec3 vPosition;  
in vec3 vNormal;  

out vec4 out_color; 

uniform vec3 u_Light;

void main(void)
{
    vec4 color = vec4(0.8, 0.8, 0.8, 1.0); 
    vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);
    vec3 fNormal = normalize(vNormal);
    float cosTheta = max(0.0, dot(normalize(u_Light - vPosition), fNormal));
    out_color = cosTheta * color + ambient;
    out_color.a = 1.0; 
}
#endif
