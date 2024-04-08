
//! \file compute_buffer.glsl exemple compute shader + buffers

#version 430

#ifdef COMPUTE_SHADER

layout( std430, binding= 0 ) readonly buffer inputData
{
	int a[];
};

layout( std430, binding= 1 ) buffer outputData
{
	int b[];
};

uniform int number;
uniform int v;
layout( local_size_x=256 ) in;
void main( )
{
	uint ID= gl_GlobalInvocationID.x;
	if(ID < a.length())
	{
		if(number > v)
			atomicAdd( b[ID], a[ID]+number );
	}
}

#endif
