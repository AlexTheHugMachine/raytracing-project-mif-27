
#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 mpMatrix;
out vec4 decal_position;
out vec2 vertex_texcoord;
out vec3 vertex_normal;

void main( )
{
    gl_Position= mvpMatrix * vec4(position, 1);
    vertex_normal = mat3(mpMatrix) * normal;
    vertex_texcoord= texcoord;
}

#endif


#ifdef FRAGMENT_SHADER
out vec4 fragment_color;

in vec2 vertex_texcoord;
in vec3 vertex_normal;

uniform int level_number;
uniform vec3 vertex_direction;
uniform sampler2D texture0;

void main( )
{
    float cos_theta = max(dot(normalize(vertex_normal), normalize(vertex_direction)), 0.0); // Loi de Lambert-Cosinus
    float quantized_cos_theta = floor(cos_theta * level_number) / level_number;
    vec4 color= texture(texture0, vertex_texcoord);
    if (color.a < 0.5) discard;
    fragment_color= color * quantized_cos_theta;
}

#endif
