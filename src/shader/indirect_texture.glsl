
//! \file indirect.glsl

#version 430

#ifdef VERTEX_SHADER

#extension GL_ARB_shader_draw_parameters : require

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;
out vec3 vertex_position;
out vec2 vertex_texcoord;

uniform mat4 modelMatrix;
uniform mat4 vpMatrix;
uniform mat4 viewMatrix;

// row_major : organisation des matrices par lignes...
layout(binding= 0, row_major, std430) readonly buffer modelData
{
    mat4 objectMatrix[];
};

void main( )
{
    gl_Position= vpMatrix * objectMatrix[gl_DrawIDARB] * modelMatrix * vec4(position, 1);
        
    // position dans le repere camera
    vertex_position= vec3(viewMatrix * objectMatrix[gl_DrawIDARB] * modelMatrix * vec4(position, 1));
    vertex_texcoord= texcoord;
}
#endif


#ifdef FRAGMENT_SHADER

in vec3 vertex_position;
in vec2 vertex_texcoord;

uniform sampler2D texture0;
uniform int level_number;

out vec4 fragment_color;

void main( )
{
    // vec4 color= vec4(1, 0.8, 0, 1);
    // recalcule la normale geometrique du triangle, dans le repere camera
    vec3 t= normalize(dFdx(vertex_position));
    vec3 b= normalize(dFdy(vertex_position));
    vec3 normal= normalize(cross(t, b));

    // float cos_theta = max(dot(normalize(vertex_normal), normalize(vertex_direction)), 0.0); // Loi de Lambert-Cosinus
    
    // matiere diffuse...
    float cos_theta= max(0.0, normal.z);
    float quantized_cos_theta = floor(cos_theta * level_number) / level_number;
    vec4 color= texture(texture0, vertex_texcoord);
    if (color.a < 0.5) discard;
    fragment_color= color * quantized_cos_theta;
    //color= color * cos_theta;
    
    //fragment_color= vec4(color.rgb, 1);
}
#endif
