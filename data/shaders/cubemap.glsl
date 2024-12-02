
//! \file draw_cubemap.glsl, dessine une cubemap a l'infini.
#version 330

#ifdef VERTEX_SHADER

void main(void)
{
    // repere projectif
    vec2 positions[3] = vec2[3]( vec2(-1,-1), vec2(3, -1), vec2(-1, 3) );
    // place le point sur le plan far / z=1... a l'infini
    gl_Position = vec4(positions[gl_VertexID], 1, 1);
}
#endif

#ifdef FRAGMENT_SHADER

uniform mat4 u_InvMatrix;
uniform vec3 u_CameraPosition;
uniform samplerCube u_Skybox;

out vec4 out_color;

void main(void)
{
    vec4 p = u_InvMatrix * vec4(gl_FragCoord.xyz, 1);
    vec3 pixel = p.xyz / p.w;
    
    vec3 direction = normalize(pixel - u_CameraPosition);
    out_color = texture(u_Skybox, direction);
}
#endif
