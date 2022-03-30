#version 430
layout(binding = 0) uniform sampler2D diffuse_tex; 
layout(location = 1) uniform float time;

layout(std140, binding = 0) uniform SceneUniforms
{
   mat4 PV;	//camera projection * view matrix
   vec4 eye_w;	//world-space eye position
};

layout(std140, binding = 1) uniform LightUniforms
{
   vec4 La;	//ambient light color
   vec4 Ld;	//diffuse light color
   vec4 Ls;	//specular light color
   vec4 light_w; //world-space light position
};

layout(std140, binding = 2) uniform MaterialUniforms
{
   vec4 ka;	//ambient material color
   vec4 kd;	//diffuse material color
   vec4 ks;	//specular material color
   float shininess; //specular exponent
};

in VertexData
{
   vec2 tex_coord;
   vec3 pw;       //world-space vertex position
   vec3 nw;   //world-space normal vector
   vec4 color;
} inData;   //block is named 'inData'

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
   float w = 20.0;
   vec2 uv = fract(inData.tex_coord*w);
   uv = abs(2.0*uv-vec2(1.0));

   vec2 fw = vec2(0.1);
   vec2 grid2 = smoothstep(vec2(0.0), fw, uv-vec2(0.1));
   float grid = min(grid2.x, grid2.y);
   
   //fragcolor = vec4(grid); return;
   //vec4 color1 = vec4(0.18, 0.56, 0.82, 1.0); // plane color
   //vec4 color2 = vec4(0.82, 0.40, 0.18, 1.0); // line color
   vec4 color1 = inData.color;
   vec4 color2 = vec4(1.0, 1.0, 1.0, 1.0);
   vec4 ktex = mix(color2, color1, grid);
   //Compute per-fragment Phong lighting
   vec4 ambient_term = ka*ktex*La;

   const float eps = 1e-8; //small value to avoid division by 0
   float d = distance(light_w.xyz, inData.pw.xyz);
   float atten = 1.0/(d*d+eps); //d-squared attenuation

   vec3 nw = normalize(inData.nw);			//world-space unit normal vector
   vec3 lw = normalize(light_w.xyz - inData.pw.xyz);	//world-space unit light vector
   vec4 diffuse_term = atten*kd*ktex*Ld*max(0.0, dot(nw, lw));

   vec3 vw = normalize(eye_w.xyz - inData.pw.xyz);	//world-space unit view vector
   vec3 rw = reflect(-lw, nw);	//world-space unit reflection vector

   vec4 specular_term = atten*ks*Ls*pow(max(0.0, dot(rw, vw)), shininess);

   fragcolor = ambient_term + diffuse_term + specular_term;

   //fragcolor = vec4(1.0, 0.0, 0.0, 1.0);
   //fragcolor = vec4(inData.tex_coord, 0.0, 1.0); //debug view tex coords
   //fragcolor = vec4(abs(nw), 1.0); //debug view normals
}

