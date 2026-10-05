// 3DViewer.cpp : Defines the entry point for the application.

#define SOKOL_IMPL
#if defined(_WIN32)
#define SOKOL_D3D11
#elif defined(__APPLE__)
#define SOKOL_METAL
#elif defined(__linux__) || defined(__unix__)
#define SOKOL_GLCORE
#endif
#define SOKOL_NO_ENTRY

#include "3DViewer.h"
#include "third_party/sokol/sokol_app.h"
#include "third_party/sokol/sokol_gfx.h"
#include "third_party/sokol/sokol_glue.h"
#include "triangle_shader.h"
#include <cglm/cglm.h>
#include "model.h"

using namespace std;

static struct {
	sg_pass_action ActionPass;
	sg_bindings binding;
	sg_pipeline pipeline;
	model triangle;
} State;

static struct {
	bool dragging = false;
	float yaw = 0.0f;        // Horizontal rotation in degrees
	float pitch = 20.0f;     // Vertical elevation in degrees
	float distance = 4.0f;   // Zoom distance from target
	vec3 target = { 0.0f, 0.0f, 0.0f }; // Target point the camera looks at
	float sensitivity = 0.25f;  // Mouse sensitivity
	float zoom_speed = 0.5f;   // Zoom speed
} Camera;

void init() {

	sg_desc Desc = {};
	Desc.environment = sglue_environment();
	sg_setup(&Desc);

	State.triangle = model_default();

	float vertices[] = {
		0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	};

	State.binding = {};
    sg_buffer_desc buffdesc = {};
	buffdesc.size = sizeof(vertices);
    buffdesc.data = SG_RANGE(vertices);
    State.binding.vertex_buffers[0] = sg_make_buffer(&buffdesc);

	sg_pipeline_desc pipedesc = {};
	pipedesc.shader = sg_make_shader(triangle_shader_desc(sg_query_backend()));
	pipedesc.layout.attrs[ATTR_triangle_pos].format = SG_VERTEXFORMAT_FLOAT3;
	pipedesc.layout.attrs[ATTR_triangle_color].format = SG_VERTEXFORMAT_FLOAT3;

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

	model_matrix(&State.triangle, model);

	glm_mat4_identity(view);
	vec3 eye = { 0.0f, 0.0f, 2.0f };
	//vec3 eye;
	eye[0] = Camera.target[0] + Camera.distance * cosf(rad_pitch) * sinf(rad_yaw); // Calculate X position based on yaw and pitch
	eye[1] = Camera.target[1] - Camera.distance * sinf(rad_pitch); // Invert Y-axis for correct camera orientation
	eye[2] = Camera.target[2] + Camera.distance * cosf(rad_pitch) * cosf(rad_yaw); // Calculate Z position based on yaw and pitch
	vec3 up = { 0.0f, 1.0f, 0.0f };
	glm_lookat(eye, Camera.target, up, view);

	float fov = glm_rad(45.0f);
	float aspect = sapp_widthf() / sapp_heightf();
	float near_plane = 0.1f;
	float far_plane = 100.0f;

	glm_perspective(fov, aspect, near_plane, far_plane, projection);

	//mat4 mvp;

	sg_pass pass = {};
	pass.action = State.ActionPass;
	pass.swapchain = sglue_swapchain();
	sg_begin_pass(&pass);

	sg_apply_pipeline(State.pipeline);
	sg_apply_bindings(&State.binding);

	triangle_params_t params ={0};
	mat4* mats[] = { &projection, &view, &model };
	glm_mat4_mulN(mats, 3, params.model);

	sg_apply_uniforms(UB_triangle_params, SG_RANGE(params));
	sg_draw(0, 3, 1);

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
	}
	else if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
		if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			Camera.dragging = false;
		}
	}
	else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
		if (Camera.dragging) {
			Camera.yaw += e->mouse_dx * Camera.sensitivity;
			Camera.pitch += e->mouse_dy * Camera.sensitivity;

			// Clamp pitch to avoid gimbal flipping at vertical extremes
			if (Camera.pitch > 89.0f)  Camera.pitch = 89.0f;
			if (Camera.pitch < -89.0f) Camera.pitch = -89.0f;
		}
	}
	else if (e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
		Camera.distance -= e->scroll_y * Camera.zoom_speed;
		if (Camera.distance < 0.5f) Camera.distance = 0.5f;
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
