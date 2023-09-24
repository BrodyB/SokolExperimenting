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
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"
#include "textured.glsl.h"
#include "Utility.h"

#define MAX_PARTICLES (4096)

// Struct for a sprite instance
struct sprite_t {
	float x, y;
	float scale;
	float velX, velY;
};

// Struct for the app state
static struct
{
	sg_pass_action pass_action;
	sg_pipeline pip;
	sg_bindings bind;
	uint8_t image_buffer[256 * 1024]; // Buffer for PNG image
	sprite_t instances[MAX_PARTICLES];
	int instance_count;
} state;

// Struct for a sprite vertex
typedef struct
{
	float x, y, z;
	float u, v;
} vertex_t;

// Forward-declare the callback when sokol_fetch is done
// loading the PNG
static void fetch_callback(const sfetch_response_t*);

static void init(void)
{
	// Setup sokol-gfx
	sg_desc gfx{};
	gfx.context = sapp_sgcontext();
	gfx.logger.func = slog_func;
	sg_setup(&gfx);

	// setup sokol-fetch with the minimal "resource limits"
	sfetch_desc_t fetch{};
	fetch.max_requests = 1;
	fetch.num_channels = 1;
	fetch.num_lanes = 1;
	fetch.logger.func = slog_func;
	sfetch_setup(&fetch);

	// pass action for clearing the framebuffer to some color
	state.pass_action = {};
	state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
	state.pass_action.colors[0].clear_value = { 0.2f, 0.2f, 0.4f, 1.0f };


	/*
	Allocate an image handle, but don't actually initialize the image yet,
	this happens later when the asynchronous file load has finished.
	Any draw calls containing such an "incomplete" image handle
	will be silently dropped.
	*/
	state.bind.fs.images[SLOT_tex] = sg_alloc_image();

	// a sampler object
	sg_sampler_desc sampler{};
	sampler.min_filter = SG_FILTER_LINEAR;
	sampler.mag_filter = SG_FILTER_LINEAR;
	state.bind.fs.samplers[SLOT_smp] = sg_make_sampler(&sampler);
	
	// Static vertex geometry buffer
	const vertex_t vertices[] = {
		//  x, y, z, u, v
		{ -0.5f, 0.5f, 1.0f, 0.0f, 0.0f},		// Top-Left
		{ 0.5f, 0.5f, 1.0f, 1.0f, 0.0f },		// Top-Right
		{ 0.5f, -0.5f, 1.0f, 1.0f, 1.0f },		// Bottom-Right
		{ -0.5f, -0.5f, 1.0f, 0.0f, 1.0f }		// Bottom-Left
	};

	sg_buffer_desc vbuffer{};
	vbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
	vbuffer.data = SG_RANGE(vertices);
	vbuffer.label = "quad-vertices";
	state.bind.vertex_buffers[0] = sg_make_buffer(&vbuffer);


	// create an index buffer for the quad
	const uint16_t indices[] = {
		0, 1, 2,  0, 2, 3
	};

	// Index buffer
	sg_buffer_desc ibuffer{};
	ibuffer.type = SG_BUFFERTYPE_INDEXBUFFER;
	ibuffer.data = SG_RANGE(indices);
	ibuffer.label = "quad-indices";
	state.bind.index_buffer = sg_make_buffer(&ibuffer);


	// Dynamic buffer for instance data
	sg_buffer_desc instbuffer{};
	instbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
	instbuffer.size = MAX_PARTICLES * sizeof(sprite_t);
	instbuffer.usage = SG_USAGE_STREAM;
	instbuffer.label = "sprite instances";
	state.bind.vertex_buffers[1] = sg_make_buffer(&instbuffer);


	// a pipeline state object (like a material basis in luxe)
	sg_pipeline_desc pip{};
	pip.cull_mode = SG_CULLMODE_BACK;
	pip.index_type = SG_INDEXTYPE_UINT16;
	pip.label = "quad-pipeline";

	pip.shader = sg_make_shader(texture_shader_desc(sg_query_backend()));
	// Vertex buffer
	pip.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3;
	pip.layout.attrs[ATTR_vs_pos].buffer_index = 0;
	pip.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
	pip.layout.attrs[ATTR_vs_texcoord0].buffer_index = 0;
	
	// Instance data buffer
	pip.layout.attrs[ATTR_vs_inst].format = SG_VERTEXFORMAT_FLOAT3;
	pip.layout.attrs[ATTR_vs_inst].buffer_index = 1;
	pip.layout.attrs[ATTR_vs_vel].format = SG_VERTEXFORMAT_FLOAT2;	// Yes, I know. The shader
	pip.layout.attrs[ATTR_vs_vel].buffer_index = 1;					// doesn't actually need velocity.
	pip.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;

	state.pip = sg_make_pipeline(&pip);

	/*
	start loading the PNG file, we don't need the returned handle since
	we can also get that inside the fetch-callback from the response
	structure.
	- NOTE that we're not using the user_data member, since all required
		state is in a global variable anyway
	*/
	char path_buf[512];
	sfetch_request_t request{};
	request.path = fileutil_get_path("data/king.png", path_buf, sizeof(path_buf));
	request.callback = fetch_callback;
	request.buffer = SFETCH_RANGE(state.image_buffer);
	sfetch_send(&request);
}

// The fetch-callback is called by sokol_fetch.h when the data is loaded,
// or when an error has occurred.
static void fetch_callback(const sfetch_response_t* response)
{
	if (response->fetched)
	{
		/* 
		The file data has been fetched, since we provided a big-enough
		buffer we can be sure that all data has been loaded here
		*/
		int png_width, png_height, num_channels;
		const int desired_channels = 4;

		stbi_uc* pixels = stbi_load_from_memory(
			(const stbi_uc*)response->data.ptr,
			(int)response->data.size,
			&png_width, &png_height,
			&num_channels, desired_channels);

		if (pixels)
		{
			// ok, time to actually initialize the sokol-gfx texture
			sg_image_desc img{};
			img.width = png_width;
			img.height = png_height;
			img.pixel_format = SG_PIXELFORMAT_RGBA8;
			img.data.subimage[0][0].ptr = pixels;
			img.data.subimage[0][0].size = (size_t)(png_width * png_height * 4);

			sg_init_image(state.bind.fs.images[SLOT_tex], &img);
			stbi_image_free(pixels);
		}
	}
	else if (response->failed)
	{
		// if loading the file failed, set clear color to red
		state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
		state.pass_action.colors[0].clear_value = { 0.0f, 1.0f, 0.0f, 1.0f };
	}
}

/*
The frame-function is fairly boring, note that no special handling is
needed for the case where the texture isn't loaded yet.
Also note the sfetch_dowork() function, this is usually called once a
frame to pump the sokol-fetch message queues.
*/
static void frame(void)
{
	// Pump the sokol-fetch message queues, and invoke response callbacks
	sfetch_dowork();

	float delta_time = (float)(sapp_frame_duration());

	// Emit new particles
	for (int i = 0; i < 10; ++i)
	{
		if (state.instance_count < MAX_PARTICLES)
		{
			float speed = random(100.0f, 300.0f);
			float rad = random(0.0f, 360.0f) * (float)M_PI / 180.0f;

			state.instances[state.instance_count] = {
				sapp_widthf() * 0.5f, sapp_heightf() * 0.5f, random(96.0f, 148.0f),  // X, Y, Scale
				cos(rad) * speed, sin(rad) * speed
			};

			++state.instance_count;
		}
		else
		{
			break;
		}
	}

	// Update particles
	for (int i = 0; i < state.instance_count; ++i)
	{
		state.instances[i].x += state.instances[i].velX * delta_time;
		state.instances[i].y += state.instances[i].velY * delta_time;
		const float scale = state.instances[i].scale * 0.5f;

		if (state.instances[i].x <= scale)
		{
			state.instances[i].x = scale + 0.01f;
			state.instances[i].velX *= -1.0f;
		}
		else if (state.instances[i].x >= sapp_widthf() - scale)
		{
			state.instances[i].x = sapp_widthf() - scale - 0.01f;
			state.instances[i].velX *= -1.0f;
		}

		if (state.instances[i].y <= scale)
		{
			state.instances[i].y = scale + 0.01f;
			state.instances[i].velY *= -1.0f;
		}
		else if (state.instances[i].y >= sapp_heightf() - (scale * 0.5f))
		{
			state.instances[i].y = sapp_heightf() - (scale * 0.5f) - 0.01f;
			state.instances[i].velY *= -1.0f;
		}
	}

	// Update the instance data
	sg_range data = {};
	data.ptr = state.instances;
	data.size = (size_t)state.instance_count * sizeof(sprite_t);
	sg_update_buffer(state.bind.vertex_buffers[1], &data);

	//
	// Perform frame drawing operations
	//

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
	sg_apply_pipeline(state.pip); // Set the material type (i.e. opaque texture material)
	sg_apply_bindings(&state.bind); // The image, vertices, and indexes
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));

	// Draw the sprite
	sg_draw(0, 6, MAX_PARTICLES); // Base element, Number of elements, instances

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