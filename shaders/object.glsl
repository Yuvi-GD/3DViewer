// object.glsl
@header #include "cglm/cglm.h"
@ctype mat4 mat4

@vs vertext_shader

layout(binding=0) uniform vs_params {
    mat4 mvp;
    mat4 model;
};

in vec4 pos;
in vec3 normal;
out vec3 world_normal;

void main(){
	gl_Position = mvp * pos;

	// Transform the normal so it rotates correctly with the 3D model
    world_normal = (model* vec4(normal, 0.0)).xyz;
}
@end

@fs fragment_shader

layout(binding=1) uniform fs_params {
    vec3 light_dir;
    vec3 base_color;
};

in vec3 world_normal;
out vec4 frag_color;

void main(){
    vec3 wn_n = normalize(world_normal);
    vec3 ld_n = normalize(light_dir);

    // The dot product calculates the light intensity.
    // max(..., 0.2) acts as "ambient" light so the dark side isn't pitch black.
    float diff = max(dot(wn_n,ld_n),0.2);

	frag_color = vec4(base_color*diff,1.0);
}
@end

@program object vertext_shader fragment_shader