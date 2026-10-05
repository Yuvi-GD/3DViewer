// object.glsl
@header #include "cglm/cglm.h"
@ctype mat4 mat4

@vs vertext_shader

in vec4 pos;
in vec3 normal;
out vec4 color;

layout(binding=0) uniform vs_params{
mat4 mvp;
};

void main(){
	gl_Position = mvp * pos;
	// Map normal directions to RGB colors
    color = vec4(normal * 0.5 + 0.5, 1.0);
}
@end

@fs fragment_shader

in vec4 color;
out vec4 frag_color;

void main(){
	frag_color = color;
}
@end

@program object vertext_shader fragment_shader