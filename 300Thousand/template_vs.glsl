#version 430
layout(location = 0) uniform mat4 M;
layout(location = 1) uniform float delta_time;

layout(std140, binding = 0) uniform SceneUniforms
{
	mat4 PV;
	vec4 eye_w;
};

in vec3 pos_attrib; //this variable holds the position of mesh vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  
layout(location = 3) in mat4 model_matrix;  // 3 4 5 6

out VertexData
{
	vec2 tex_coord;
	vec3 pw;
	vec3 nw;
} outData;

void main(void)
{
	gl_Position = PV * model_matrix * M * vec4(pos_attrib, 1.0); //transform vertices and send result into pipeline

	outData.tex_coord = tex_coord_attrib; //send tex_coord to fragment shader
	outData.pw = vec3(M * vec4(pos_attrib, 1.0));
	outData.nw = vec3(M * vec4(normal_attrib, 0.0));
}

