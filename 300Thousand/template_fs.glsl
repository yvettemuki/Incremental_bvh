#version 430
layout(binding = 0) uniform sampler2D diffuse_tex;

layout(location = 1) uniform float delta_time;

layout(std140, binding = 0) uniform SceneUniforms
{
	mat4 PV;
	vec4 eye_w;
};

layout(std140, binding = 1) uniform LightUniforms
{
	vec4 La;
	vec4 Ld;
	vec4 Ls;
	vec4 light_w;
};

layout(std140, binding = 2) uniform MaterialUniforms
{
	vec4 ka;
	vec4 kd;
	vec4 ks;
	float shininess;
};

in VertexData
{
	vec2 tex_coord;
	vec3 pw;
	vec3 nw;
} inData; 

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
	fragcolor = texture(diffuse_tex, inData.tex_coord);
}




















