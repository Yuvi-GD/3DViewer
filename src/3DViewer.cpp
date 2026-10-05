// 3DViewer.cpp : Defines the entry point for the application.
#include "3DViewer.h"
#include "model.h"

#define SOKOL_IMPL
#if defined(_WIN32)
#define SOKOL_D3D11
#elif defined(__APPLE__)
#define SOKOL_METAL
#elif defined(__linux__) || defined(__unix__)
#define SOKOL_GLCORE
#endif
#define SOKOL_NO_ENTRY

#include "third_party/sokol/sokol_app.h"
#include "third_party/sokol/sokol_gfx.h"
#include "third_party/sokol/sokol_glue.h"
#include "object_shader.h"
#include <cglm/cglm.h>


static struct {
	sg_pass_action ActionPass;
	mesh_data loaded_mesh;
	sg_pipeline pipeline;
	model object;
} State;

static struct {
	bool dragging = false;
	bool panning = false;
	float yaw = 0.0f;        // Horizontal rotation in degrees
	float pitch = 20.0f;     // Vertical elevation in degrees
	float distance = 4.0f;   // Zoom distance from target
	vec3 target = { 0.0f, 0.0f, 0.0f }; // Target point the camera looks at
	float sensitivity = 0.25f;  // Mouse sensitivity
	float zoom_speed = 0.5f;   // Zoom speed

	// Store defaults for the reset hotkey
	vec3 default_target = { 0.0f, 0.0f, 0.0f };
	float default_distance = 4.0f;
} Camera;

void init() {

	sg_desc Desc = {};
	Desc.environment = sglue_environment();
	sg_setup(&Desc);

	State.object = model_default();

	State.loaded_mesh = load_obj_mesh("model.obj");

	// 1. Calculate the center of the bounding box
	Camera.target[0] = (State.loaded_mesh.max_x + State.loaded_mesh.min_x) * 0.5f;
	Camera.target[1] = (State.loaded_mesh.max_y + State.loaded_mesh.min_y) * 0.5f;
	Camera.target[2] = (State.loaded_mesh.max_z + State.loaded_mesh.min_z) * 0.5f;

	// 2. Find the largest dimension (X, Y, or Z) to figure out how far to zoom out
	float size_x = State.loaded_mesh.max_x - State.loaded_mesh.min_x;
	float size_y = State.loaded_mesh.max_y - State.loaded_mesh.min_y;
	float size_z = State.loaded_mesh.max_z - State.loaded_mesh.min_z;

	float max_size = size_x;
	if (size_y > max_size) max_size = size_y;
	if (size_z > max_size) max_size = size_z;

	// 3. Set the distance based on the max size (1.5x is a good framing multiplier for a 45-degree FOV)
	Camera.distance = max_size * 1.5f;

	// Scale the pan and zoom speeds so they feel right for massive or tiny objects
	Camera.zoom_speed = max_size * 0.1f;

	// 4. Save these as the default so the '.' key can reset to them
	glm_vec3_copy(Camera.target, Camera.default_target);
	Camera.default_distance = Camera.distance;

	sg_pipeline_desc pipedesc = {};
	pipedesc.shader = sg_make_shader(object_shader_desc(sg_query_backend()));
	pipedesc.layout.attrs[ATTR_object_pos].format = SG_VERTEXFORMAT_FLOAT3;
	pipedesc.layout.attrs[ATTR_object_normal].format = SG_VERTEXFORMAT_FLOAT3;
	pipedesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
	pipedesc.depth.write_enabled = true;
	pipedesc.index_type = SG_INDEXTYPE_UINT16;
	pipedesc.cull_mode = SG_CULLMODE_BACK;
	pipedesc.face_winding = SG_FACEWINDING_CCW;
	State.pipeline = sg_make_pipeline(&pipedesc);


    // set pass action (explicit field assignments to be safe in C++)
    State.ActionPass = {};
    State.ActionPass.colors[0].load_action = SG_LOADACTION_CLEAR;
	State.ActionPass.colors[0].clear_value = { 0.125, 0.125, 0.125, 1.0 };
}

void frame() {

	float deltaTime = (float)sapp_frame_duration();

	// Convert yaw/pitch/distance to Cartesian (X, Y, Z) coordinates
	float rad_yaw = glm_rad(Camera.yaw);
	float rad_pitch = glm_rad(Camera.pitch);

	mat4 model, view, projection;

	model_matrix(&State.object, model);

	glm_mat4_identity(view);
	vec3 eye = { 0.0f, 0.0f, 2.0f };
	eye[0] = Camera.target[0] - Camera.distance * cosf(rad_pitch) * sinf(rad_yaw); // Calculate X position based on yaw and pitch
	eye[1] = Camera.target[1] + Camera.distance * sinf(rad_pitch); // Invert Y-axis for correct camera orientation
	eye[2] = Camera.target[2] + Camera.distance * cosf(rad_pitch) * cosf(rad_yaw); // Calculate Z position based on yaw and pitch
	vec3 up = { 0.0f, 1.0f, 0.0f };
	glm_lookat(eye, Camera.target, up, view);

	float fov = glm_rad(45.0f);
	float aspect = sapp_widthf() / sapp_heightf();
	float near_plane = 0.1f;
	float far_plane = 100.0f;

	glm_perspective(fov, aspect, near_plane, far_plane, projection);


	sg_pass pass = {};
	pass.action = State.ActionPass;
	pass.swapchain = sglue_swapchain();
	sg_begin_pass(&pass);

	sg_apply_pipeline(State.pipeline);
	sg_apply_bindings(&State.loaded_mesh.bind);

	vs_params_t vs_params ={0};
	glm_mat4_copy(model, vs_params.model); // Send the raw model matrix to rotate the normals

	mat4* mats[] = { &projection, &view, &model };
	glm_mat4_mulN(mats, 3, vs_params.mvp);
	
	sg_apply_uniforms(UB_vs_params, SG_RANGE(vs_params));

	// Fragment Shader Uniforms (Light & Color)
	fs_params_t fs_params = {0};
	// Light coming from the top-right-front
	fs_params.light_dir[0] = 1.0f;
	fs_params.light_dir[1] = 1.0f;
	fs_params.light_dir[2] = 0.5f;
	// Base color: Light Grey
	fs_params.base_color[0] = 0.8f;
	fs_params.base_color[1] = 0.8f;
	fs_params.base_color[2] = 0.8f;

	sg_apply_uniforms(UB_fs_params, SG_RANGE(fs_params));

	// Draw model
	sg_draw(0, State.loaded_mesh.index_count, 1);

	sg_end_pass();
	sg_commit();
}

void cleanup() {
	sg_shutdown();
}

void event(const sapp_event* e) {
	if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
		if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			Camera.dragging = true;
		}
		else if (e->mouse_button == SAPP_MOUSEBUTTON_MIDDLE) {
			Camera.panning = true;
		}
	}
	else if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
		if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			Camera.dragging = false;
		}
		else if (e->mouse_button == SAPP_MOUSEBUTTON_MIDDLE) {
			Camera.panning = false;
		}
	}
	else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
		if (Camera.dragging) {
			Camera.yaw += e->mouse_dx * Camera.sensitivity;
			Camera.pitch += e->mouse_dy * Camera.sensitivity;

			if (Camera.pitch > 89.0f)  Camera.pitch = 89.0f;
			if (Camera.pitch < -89.0f) Camera.pitch = -89.0f;
		}
		else if (Camera.panning) {
			float rad_yaw = glm_rad(Camera.yaw);
			float rad_pitch = glm_rad(Camera.pitch);

			// 1. Calculate the camera's "Right" vector
			vec3 right = { cosf(rad_yaw), 0.0f, sinf(rad_yaw) };

			// 2. Calculate the camera's "Local Up" vector
			vec3 up = {
				-sinf(rad_yaw) * sinf(rad_pitch),
				cosf(rad_pitch),
				cosf(rad_yaw) * -sinf(rad_pitch)
			};

			// 3. Scale pan speed by distance so panning feels consistent whether zoomed in or out
			float pan_speed = 0.0015f * Camera.distance;

			// 4. Move the target along those axes
			Camera.target[0] += (right[0] * -e->mouse_dx + up[0] * e->mouse_dy) * pan_speed;
			Camera.target[1] += (right[1] * -e->mouse_dx + up[1] * e->mouse_dy) * pan_speed;
			Camera.target[2] += (right[2] * -e->mouse_dx + up[2] * e->mouse_dy) * pan_speed;
		}
	}
	else if (e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
		// Zooming is just changing the distance from the target
		Camera.distance -= e->scroll_y * Camera.zoom_speed;
		if (Camera.distance < 0.5f) Camera.distance = 0.5f;
	}
	else if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
		// Reset the camera target to 0,0,0 when the dot/period key is pressed
		if (e->key_code == SAPP_KEYCODE_PERIOD || e->key_code == SAPP_KEYCODE_KP_DECIMAL) {
			Camera.target[0] = 0.0f;
			Camera.target[1] = 0.0f;
			Camera.target[2] = 0.0f;

			// reset the zoom and rotation here if you want a complete reset
			Camera.distance = Camera.default_distance;
			 Camera.yaw = 0.0f;
			 Camera.pitch = 20.0f;
		}
	}
}

int main()
{
	sapp_desc Desc = {
	.init_cb = init,
	.frame_cb = frame,
	.cleanup_cb = cleanup,
	.event_cb = event,
	.width = 800,
	.height = 600,
	.window_title = "3DViewer",
	};

	sapp_run(&Desc);
	return 0;
}
