#version 430
layout(location = 0) uniform mat4 M;
layout(location = 1) uniform float delta_time;
layout(location = 2) uniform int type;

layout(std140, binding = 0) uniform SceneUniforms
{
	mat4 PV;
	vec4 eye_w;
};

layout(location = 0) in vec3 pos_attrib; //this variable holds the position of mesh vertices
layout(location = 1) in vec2 tex_coord_attrib;
layout(location = 2) in vec3 normal_attrib;  
layout(location = 3) in mat4 model_matrix;  // 3 4 5 6

// quad for the arena ground plane
const vec4 quad[4] = vec4[] (
	vec4(-1.0, 0.0, 1.0, 1.0), 
	vec4(-1.0, 0.0, -1.0, 1.0), 
	vec4( 1.0, 0.0, 1.0, 1.0), 
	vec4( 1.0, 0.0, -1.0, 1.0)
);

out VertexData
{
	vec2 tex_coord;
	vec3 pw;
	vec3 nw;
} outData;

void main(void)
{
	if (type == 1)
	{
		// objects 
		gl_Position = PV * M * model_matrix * vec4(pos_attrib, 1.0); //transform vertices and send result into pipeline

		outData.tex_coord = tex_coord_attrib; //send tex_coord to fragment shader
		outData.pw = vec3(M * vec4(pos_attrib, 1.0));
		outData.nw = vec3(M * vec4(normal_attrib, 0.0));
	}

	if (type == 2)
	{
		// bounding box
		gl_Position = PV * M * vec4(pos_attrib, 1.0);
	}

	if (type == 3)
	{
		// arena plane
		gl_Position = PV * M * quad[gl_VertexID];
		outData.tex_coord = 0.5 * (quad[gl_VertexID].xy + vec2(1.0));
		outData.pw = vec3(M * quad[gl_VertexID]);
		outData.nw = vec3(M * vec4(0.0, 1.0, 0.0, 0.0));
	}
	
}

