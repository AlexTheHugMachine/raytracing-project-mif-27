
#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
 
uniform mat4 mvpMatrix;
uniform mat4 decalMatrix;
 
out vec4 decal_position;
 
void main( )
{
    gl_Position= mvpMatrix * vec4(position, 1);
    decal_position= decalMatrix * vec4(position, 1);
}

#endif


#ifdef FRAGMENT_SHADER
out vec4 fragment_color;
 
uniform sampler2D decal;
in vec4 decal_position;
 
void main( )
{
    // teste les coordonnées du decal
    vec3 texcoord= decal_position.xyz / decal_position.w;
    vec3 decal_color= texture(decal, texcoord.xy).rgb;
    if(texcoord.x < 0 || texcoord.x > 1)
        decal_color.g= 1;
    if(texcoord.y < 0 || texcoord.y > 1)
        decal_color.r= 1;
    if(texcoord.z < 0 || texcoord.z > 1)
        decal_color.b= 1;
    
    fragment_color= vec4(decal_color, 1);
}


#endif
