#version 330

#ifdef VERTEX_SHADER
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexcoord;
layout(location=2) in vec3 aNormal;

uniform mat4 u_MvpMatrix; 
uniform mat4 uMvMatrix; 
uniform mat4 uNormalMatrix; 

out vec3 vPosition; 
out vec2 vTexcoord; 
out vec3 vNormal; 

void main(void)
{
    vNormal = mat3(uNormalMatrix) * aNormal; 
    vPosition = vec3(uMvMatrix * vec4(aPosition, 1.0));
    vTexcoord = aTexcoord;
    gl_Position = u_MvpMatrix * vec4(aPosition, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER
in vec3 vPosition;  
in vec2 vTexcoord;  
in vec3 vNormal;  

out vec4 out_color; 

uniform vec3 uLight;
uniform sampler2D uTexture; 

void main(void)
{
  vec4 color = texture(uTexture, vTexcoord);

  vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);
  vec3 fNormal = normalize(vNormal);
  float cosTheta = max(0.0, dot(normalize(uLight - vPosition), fNormal));
  out_color = cosTheta * color + ambient;
  out_color.a = 1.0; 
}
#endif
