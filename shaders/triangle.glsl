// triangle.glsl
@header #include "cglm/cglm.h"
@ctype mat4 mat4

@vs vertext_shader

in vec3 pos;
in vec3 color;
out vec3 outcolor;

layout(binding=0) uniform triangle_params{
mat4 model;
};

void main(){
	gl_Position = model* vec4(pos, 1.0);
	outcolor=color;
}
@end

@fs fragment_shader

in vec3 outcolor;
out vec4 frag_color;

void main(){
	frag_color = vec4(outcolor,1.0);
}
@end

@program triangle vertext_shader fragment_shader