#version 330
 
#ifdef VERTEX_SHADER
uniform mat4 mvpMatrix; // la transformation à appliquer aux sommets, mot-clé : uniform
uniform mat4 mvMatrix;
layout (location=0) in vec3 position;       // la position du ieme sommet, mot-clé : in
layout (location=2) in vec3 normal;
layout (location=4) in uint material;
//layout(location= 1) in vec2 texcoord;
out vec3 vertex_normal;
//out vec2 vertex_texcoord;
flat out uint vertex_material;
 
void main( )
{
    gl_Position= mvpMatrix * vec4(position, 1);
    vertex_normal = mat3(mvMatrix) * normal;
    vertex_material=material;
    //vertex_texcoord= texcoord;
}
#endif
 
#ifdef FRAGMENT_SHADER
uniform vec3 vertex_direction;
uniform vec4 vertex_color;
uniform int level_number;
//uniform sampler2D texture0;
flat in uint vertex_material;
in vec3 vertex_normal;
//in vec2 vertex_texcoord;
out vec4 fragment_color;

void main( )
{
    float cos_theta = max(dot(normalize(vertex_normal), normalize(vertex_direction)), 0.0); // Loi de Lambert-Cosinus
    float quantized_cos_theta = floor(cos_theta * level_number) / level_number;
    //vec4 color= texture(texture0, vertex_texcoord);
    //fragment_color = color;
    fragment_color = vec4(vec3(quantized_cos_theta) * vertex_color.rgb, vertex_color.a);
}
#endif
