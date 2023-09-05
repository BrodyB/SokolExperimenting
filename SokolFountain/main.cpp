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

// File path utility function
const char* fileutil_get_path(const char* filename, char* buf, size_t buf_size)
{
	snprintf(buf, buf_size, "%s", filename);
	return buf;
}

const float normalize_x(float x)
{
	return x * (2.0f / sapp_widthf());
}

const float normalize_y(float y)
{
	return y * (2.0f / sapp_heightf());
}

// Struct for the app state
static struct
{
	sg_pass_action pass_action;
	sg_pipeline pip;
	sg_bindings bind;
	uint8_t file_buffer[256 * 1024]; // Buffer for PNG image
} state;

// Struct for a sprite instance
struct sprite_t {
	float x;
	float y;
	float scale;
};

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
	
	// quad vertex buffer with packed texcoords
	float w = normalize_x(256);
	float h = normalize_y(256.0f);
	float spx = normalize_x(16.0f);
	float spy = normalize_y(16.0f);
	const float spriteZ = 0.0f;
	const float x1 = -1.0f + w;
	const float x2 = x1 + (w * 2.0f) + spx;
	const float x3 = x2 + (w * 2.0f) + spx;
	const float y1 = 1.0f - h;
	const float y2 = y1 - (h * 2.0f) - spy;

	const sprite_t sprite_data[] = {
		{ x1, y1, 0.25f },
		{ x2, y1, 0.5f },
		{ x3, y1, 0.33f},
		{ x1, y2, 0.8f },
		{ x2, y2, 1.0f },
		{ x3, y2, 0.75f },
	};

	const vertex_t vertices[] = {
		//  x, y, z, u, v
		{ -w, h, spriteZ, 0.0f, 0.0f},		// Top-Left
		{ w, h, spriteZ, 1.0f, 0.0f },		// Top-Right
		{ w, -h, spriteZ, 1.0f, 1.0f },		// Bottom-Right
		{ -w, -h, spriteZ, 0.0f, 1.0f }		// Bottom-Left
	};

	sg_buffer_desc vbuffer{};
	vbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
	vbuffer.data = SG_RANGE(vertices);
	vbuffer.label = "quad-vertices";
	state.bind.vertex_buffers[0] = sg_make_buffer(&vbuffer);

	sg_buffer_desc instbuffer{};
	instbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
	instbuffer.data = SG_RANGE(sprite_data);
	instbuffer.label = "sprite instances";
	state.bind.vertex_buffers[1] = sg_make_buffer(&instbuffer);

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

	// a pipeline state object (like a material basis in luxe)
	sg_pipeline_desc pip{};
	pip.cull_mode = SG_CULLMODE_NONE;
	pip.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
	pip.depth.write_enabled = true;
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
	request.buffer = SFETCH_RANGE(state.file_buffer);
	sfetch_send(&request);
}

// The fetch-callback is called by sokol_fetch.h when the data is loaded,
// or when an error has occurred.
static void fetch_callback(const sfetch_response_t* response)
{
	if (response->fetched)
	{
		/* the file data has been fetched, since we provided a big-enough
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
	// pump the sokol-fetch message queues, and invoke response callbacks
	sfetch_dowork();

	//
	// Perform frame drawing operations
	//

	// set render target
	sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
	// Set the sprite render settings
	sg_apply_pipeline(state.pip); // Set the material type (i.e. opaque texture material)
	sg_apply_bindings(&state.bind); // The image, vertices, and indexes
	// Draw the sprite
	sg_draw(0, 6, 6); // Base element, Number of elements, instances

	// To do sprite instances
		// 1. Up the instance count
			// Need to have per-instance data, like position

			// Image images[] // give me several images, as unique handles
				// Images can be different sizes
			// ImageArray images; // give me one handle, but with several slices
				// All images must be the same dimensions

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
	desc.window_title = "PNG QUAD";
	desc.icon.sokol_default = true;
	desc.logger.func = slog_func;

	return desc;
}