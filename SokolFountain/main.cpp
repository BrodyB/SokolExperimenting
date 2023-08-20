//------------------------------------------------------------------------------
//  loadpng-sapp.c
//  Asynchronously load a png file via sokol_fetch.h, decode via stb_image.h
//  (this is non-perfect since it happens on the main thread)
//  and create a sokol-gfx texture from the decoded pixel data.
//
//  The CMakeLists.txt entry for loadpng-sapp.c also demonstrates the
//  sokol_file_copy() macro to copy assets into the fips deployment directory.
//
//  This is a modified version of texcube-sapp.c
//------------------------------------------------------------------------------
#include <iostream>
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "../libs/HandmadeMath.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_fetch.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"
#include "loadpng-sapp.glsl.h"

const char* fileutil_get_path(const char* filename, char* buf, size_t buf_size) {
    snprintf(buf, buf_size, "%s", filename);
    return buf;
}

static struct {
    float rx, ry; // Rotation x/y for spinning
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
    uint8_t file_buffer[256 * 1024]; // Buffer for PNG image
} state;

typedef struct {
    float x, y, z;
    int16_t u, v;
} vertex_t;

static void fetch_callback(const sfetch_response_t*);

static void init(void) {
    // setup sokol-gfx and the optional debug-ui
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
    

    /* Allocate an image handle, but don't actually initialize the image yet,
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

    // cube vertex buffer with packed texcoords
    const vertex_t vertices[] = {
        // pos                  uvs
        { -1.0f, -1.0f, -1.0f,      0,     0 },
        {  1.0f, -1.0f, -1.0f,  32767,     0 },
        {  1.0f,  1.0f, -1.0f,  32767, 32767 },
        { -1.0f,  1.0f, -1.0f,      0, 32767 },

        { -1.0f, -1.0f,  1.0f,      0,     0 },
        {  1.0f, -1.0f,  1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        { -1.0f,  1.0f,  1.0f,      0, 32767 },

        { -1.0f, -1.0f, -1.0f,      0,     0 },
        { -1.0f,  1.0f, -1.0f,  32767,     0 },
        { -1.0f,  1.0f,  1.0f,  32767, 32767 },
        { -1.0f, -1.0f,  1.0f,      0, 32767 },

        {  1.0f, -1.0f, -1.0f,      0,     0 },
        {  1.0f,  1.0f, -1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        {  1.0f, -1.0f,  1.0f,      0, 32767 },

        { -1.0f, -1.0f, -1.0f,      0,     0 },
        { -1.0f, -1.0f,  1.0f,  32767,     0 },
        {  1.0f, -1.0f,  1.0f,  32767, 32767 },
        {  1.0f, -1.0f, -1.0f,      0, 32767 },

        { -1.0f,  1.0f, -1.0f,      0,     0 },
        { -1.0f,  1.0f,  1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        {  1.0f,  1.0f, -1.0f,      0, 32767 },
    };

    sg_buffer_desc vbuffer{};
    vbuffer.data = SG_RANGE(vertices);
    vbuffer.label = "cube-vertices";
    state.bind.vertex_buffers[0] = sg_make_buffer(&vbuffer);


    // create an index buffer for the cube
    const uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };

    sg_buffer_desc ibuffer{};
    ibuffer.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuffer.data = SG_RANGE(indices);
    ibuffer.label = "cube-indices";
    state.bind.index_buffer = sg_make_buffer(&ibuffer);


    // a pipeline state object
    sg_pipeline_desc pip{};
    pip.shader = sg_make_shader(loadpng_shader_desc(sg_query_backend()));
    pip.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3;
    pip.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N;
    pip.index_type = SG_INDEXTYPE_UINT16;
    pip.cull_mode = SG_CULLMODE_BACK;
    pip.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    pip.depth.write_enabled = true;
    pip.label = "cube-pipeline";
    state.pip = sg_make_pipeline(&pip);

    /* start loading the PNG file, we don't need the returned handle since
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

/* The fetch-callback is called by sokol_fetch.h when the data is loaded,
   or when an error has occurred.
*/
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
        state.pass_action.colors[0].clear_value = { 1.0f, 0.0f, 0.0f, 1.0f };
    }
}

/* The frame-function is fairly boring, note that no special handling is
   needed for the case where the texture isn't loaded yet.
   Also note the sfetch_dowork() function, this is usually called once a
   frame to pump the sokol-fetch message queues.
*/
static void frame(void) {
    // pump the sokol-fetch message queues, and invoke response callbacks
    sfetch_dowork();

    // compute model-view-projection matrix for vertex shader
    const float t = (float)(sapp_frame_duration() * 60.0);
    hmm_mat4 proj = HMM_Perspective(60.0f, sapp_widthf() / sapp_heightf(), 0.01f, 10.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
    vs_params_t vs_params;
    state.rx += 1.0f * t; state.ry += 2.0f * t;
    hmm_mat4 rxm = HMM_Rotate(state.rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(state.ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
    vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    sg_draw(0, 36, 1);
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    sfetch_shutdown();
    sg_shutdown();
}


// This is the actual main function that's called at start
sapp_desc sokol_main(int argc, char* argv[]) {
    sapp_desc desc{};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.width = 800;
    desc.height = 600;
    desc.sample_count = 4;
    desc.window_title = "PNG CUBE";
    desc.icon.sokol_default = true;
    desc.logger.func = slog_func;

    return desc;
}