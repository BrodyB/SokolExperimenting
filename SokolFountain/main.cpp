//------------------------------------------------------------------------------
//  Modification of loadpng-sapp.c
//
//  Asynchronously load a png file via sokol_fetch.h, decode via stb_image.h
//  (this is non-perfect since it happens on the main thread)
//  and create a sokol-gfx texture from the decoded pixel data.
//
//  Trying to take this from a spinning cube to just showing a textured quad,
//  and then eventually a lot of textured quads
//------------------------------------------------------------------------------
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_CPP_MODE
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#define _USE_MATH_DEFINES
#include <math.h>
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_fetch.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "textured.glsl.h"
#include "ParticleSystem.h"

#define MAX_PARTICLES (4096)

// Struct for the app state
static struct
{
	sg_pass_action pass_action;
	ParticleSystem particles;
} state;

static void init(void)
{
	// Setup sokol-gfx
	sg_desc gfx{};
	gfx.context = sapp_sgcontext();
	gfx.logger.func = slog_func;
	sg_setup(&gfx);

	// pass action for clearing the framebuffer to some color
	state.pass_action = {};
	state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
	state.pass_action.colors[0].clear_value = { 0.2f, 0.2f, 0.4f, 1.0f };

	// Static vertex geometry buffer
	std::vector<vertex_t> vertices = {
		//  x, y, z, u, v
		{ -0.5f, 0.5f, 1.0f, 0.0f, 0.0f},		// Top-Left
		{ 0.5f, 0.5f, 1.0f, 1.0f, 0.0f },		// Top-Right
		{ 0.5f, -0.5f, 1.0f, 1.0f, 1.0f },	// Bottom-Right
		{ -0.5f, -0.5f, 1.0f, 0.0f, 1.0f }	// Bottom-Left
	};

	// create an index buffer for the quad
	std::vector<uint32_t> indices = {
		0, 1, 2,  0, 2, 3
	};

	state.particles.AddEmitter(&vertices, &indices);
}

/*
The frame-function is fairly boring, note that no special handling is
needed for the case where the texture isn't loaded yet.
Also note the sfetch_dowork() function, this is usually called once a
frame to pump the sokol-fetch message queues.
*/
static void frame(void)
{
	float delta_time = static_cast<float>(sapp_frame_duration());

	// compute model-view-projection matrix for vertex shader
	hmm_mat4 proj = HMM_Orthographic(0.0f, sapp_widthf(), 0.0f, sapp_heightf(), 0.0f, 5.0f);
	hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
	vs_params_t vs_params;
	hmm_mat4 rxm = HMM_Rotate(0.0f, HMM_Vec3(1.0f, 0.0f, 0.0f));
	hmm_mat4 rym = HMM_Rotate(0.0f, HMM_Vec3(0.0f, 1.0f, 0.0f));
	hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
	vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

	// set render target
	sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
	// Set the sprite render settings

	sg_end_pass();

	// Finish the drawing operation
	sg_commit();
}

// Cleanup on app shutdown
static void cleanup(void)
{
	sfetch_shutdown();
	sg_shutdown();
}


// This is the actual main function that's called at start
sapp_desc sokol_main(int argc, char* argv[])
{
	sapp_desc desc{};
	desc.init_cb = init;
	desc.frame_cb = frame;
	desc.cleanup_cb = cleanup;
	desc.width = 1280;
	desc.height = 720;
	desc.window_title = "Instance Flood";
	desc.icon.sokol_default = true;
	desc.logger.func = slog_func;

	return desc;
}