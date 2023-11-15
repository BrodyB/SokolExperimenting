#include "EmitterGood.h"

#include "sokol_gfx.h"
#include "HandmadeMath.h"
#include "ParticleTypes.h"
#include "textured.glsl.h"
#include "../libs/stb/stb_image.h"
#include "Utility.h"
#include <util/sokol_debugtext.h>
#include <sokol_time.h>

EmitterGood::EmitterGood(const char* imgPath, const std::vector<vertex_t>* vertices, const std::vector<uint16_t>* indices, float durationMin, float durationMax, int32_t maxParticles)
{
    lifespanMin = durationMin;
    lifespanMax = durationMax;
    this->maxParticles = maxParticles;

    fromColor.r = 0.8f;
    fromColor.g = 0.8f;
    fromColor.b = 0.8f;
    fromColor.a = 1.0f;

    toColor.r = 0.3f;
    toColor.g = 0.3f;
    toColor.b = 0.3f;
    toColor.a = 0.0f;

    sdtx_desc_t textDesc{};
    textDesc.fonts[0] = sdtx_font_oric();
    textDesc.logger.func = slog_func;
    sdtx_setup(&textDesc);

    bindings.fs.images[SLOT_tex] = sg_alloc_image();

    sg_sampler_desc sampler{};
    sampler.min_filter = SG_FILTER_LINEAR;
    sampler.mag_filter = SG_FILTER_LINEAR;
    bindings.fs.samplers[SLOT_smp] = sg_make_sampler(&sampler);

    int width, height, channels;
    stbi_uc* img = stbi_load(imgPath, &width, &height, &channels, 4);

    if (img)
    {
        sg_image_desc desc{};
        desc.width = width;
        desc.height = height;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.data.subimage[0][0].ptr = img;
        desc.data.subimage[0][0].size = (size_t)(width * height * 4);

        sg_init_image(bindings.fs.images[SLOT_tex], &desc);
        stbi_image_free(img);
    }
    else
    {
        LOG("UNABLE TO LOAD IMAGE!");
    }


    // Create the vertex buffer
    sg_buffer_desc vbuffer = { 0 };
    vbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
    vbuffer.data = sg_range{ vertices->data(), (int32_t)vertices->size() * sizeof(vertex_t) };
    vbuffer.label = "emitter vertices";
    bindings.vertex_buffers[0] = sg_make_buffer(&vbuffer);

    // Create the index buffer
    sg_buffer_desc ibuffer = { 0 };
    ibuffer.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuffer.data = sg_range{ indices->data(), (int32_t)indices->size() * sizeof(uint16_t) }; // problem?
    ibuffer.label = "emitter indices";
    bindings.index_buffer = sg_make_buffer(&ibuffer);
    indexCount = (int32_t)indices->size();


    // Dynamic buffer for instance data
    sg_buffer_desc instbuffer = { 0 };
    instbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
    instbuffer.size = maxParticles * sizeof(ParticleData);
    instbuffer.usage = SG_USAGE_STREAM;
    instbuffer.label = "particle instances";
    bindings.vertex_buffers[1] = sg_make_buffer(&instbuffer);

    // a pipeline state object (like a material basis in luxe)
    sg_pipeline_desc pip{};
    pip.cull_mode = SG_CULLMODE_NONE;
    pip.index_type = SG_INDEXTYPE_UINT16;
    pip.label = "emitter pipeline";

    pip.shader = sg_make_shader(instancing_shader_desc(sg_query_backend()));
    pip.colors[0].blend.enabled = true;
    pip.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pip.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

    // Vertex buffer
    pip.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_pos].buffer_index = 0;
    pip.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    pip.layout.attrs[ATTR_vs_texcoord0].buffer_index = 0;

    // Instance data buffer
    pip.layout.attrs[ATTR_vs_inst_pos].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_inst_pos].buffer_index = 1;
    pip.layout.attrs[ATTR_vs_inst_col].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_inst_col].buffer_index = 1;
    pip.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    pipeline = sg_make_pipeline(&pip);
}

void EmitterGood::Start()
{
    isActive = true;
}

void EmitterGood::Stop(bool immediately)
{
    isActive = false;
}

void EmitterGood::Tick(float deltaTime, hmm_mat4 params)
{
    uint64_t oldTime = stm_now();

    EmitParticles(deltaTime);
    UpdateInstances(deltaTime);

    updateTimes[updateIndex] = stm_ms(stm_since(oldTime));
    updateMax = std::max(updateMax, updateTimes[updateIndex]);
    updateIndex += 1;
    if (updateIndex > 31) updateIndex = 0;

    // Draw emitter particles
    sg_apply_pipeline(pipeline);
    sg_apply_bindings(bindings);
    vs_params_t vs_params;
    vs_params.mvp = params;
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    sg_draw(0, indexCount, (int)particleData.size()); // Base element, Number of elements, instances

    double timeSince = 0;
    for (int i = 0; i < 32; ++i)
    {
        timeSince += updateTimes[i];
    }

    // help text
    char buffer[50];
    sprintf_s(buffer, "Good Emitter\n\n Average: %.2fms\n Highest: %.2fms", timeSince / 32, updateMax);
    sdtx_canvas(720.0f, 360.0f);
    sdtx_pos(0.5f, 0.5f);
    sdtx_puts(buffer);
    sdtx_draw();
}

void EmitterGood::SetOffsetPosition(float x, float y, float z)
{
    offsetPos[0] = x;
    offsetPos[1] = y;
    offsetPos[2] = z;
}

void EmitterGood::AddModule(IModule& mod)
{
}

void EmitterGood::EmitParticles(float deltaTime)
{
    if (!isActive) return;

    for (int i = 0; i < emissionRate; ++i)
    {
        int size = (int)particleData.size();
        int cap = (int)particleData.capacity();

        if (particleData.size() < maxParticles)
        {
            ParticleData data;
            particleData.push_back(data);

            hmm_vec4 pos;
            pos.X = offsetPos[0] + random(-500.0f, 500.0f);
            pos.Y = offsetPos[1] + random(-100.0f, 100.0f);
            pos.Z = offsetPos[2];
            pos.W = 1.0f;
            positions.push_back(pos);

            color_t col;
            colors.push_back(col);

            hmm_vec2 life;
            life.X = 0.0f;
            life.Y = random(lifespanMin, lifespanMax);
            lifetimes.push_back(life);
        }
    }
}

void EmitterGood::UpdateInstances(float deltaTime)
{
    int i = 0;

    for (i = 0; i < lifetimes.size(); ++i)
    {
        // Remove particle data if it's expired
        if (lifetimes[i].X >= lifetimes[i].Y)
        {
            int last = (int)lifetimes.size() - 1;

            ParticleData tempData = particleData[last];
            particleData[last] = particleData[i];
            particleData[i] = tempData;
            particleData.pop_back();

            hmm_vec4 tempPos = positions[last];
            positions[last] = positions[i];
            positions[i] = tempPos;
            positions.pop_back();

            color_t tempColor = colors[last];
            colors[last] = colors[i];
            colors[i] = tempColor;
            colors.pop_back();

            hmm_vec2 tempLife = lifetimes[last];
            lifetimes[last] = lifetimes[i];
            lifetimes[i] = tempLife;
            lifetimes.pop_back();
            continue;
        }

        lifetimes[i].X = std::min(lifetimes[i].X + deltaTime, lifetimes[i].Y);
    }

    for (i = 0; i < positions.size(); ++i)
    {
        positions[i].X += 0.1f * deltaTime;
        positions[i].Y += 75.0f * deltaTime;
        positions[i].Z += deltaTime;
        positions[i].W = lerp(64.0f, 360.0f, lifetimes[i].X / lifetimes[i].Y);
    }

    for (i = 0; i < colors.size(); ++i)
    {
        float percent = lifetimes[i].X / lifetimes[i].Y;
        colors[i].r = clamp(lerp(fromColor.r, toColor.r, percent), 0.0f, 1.0f);
        colors[i].g = clamp(lerp(fromColor.g, toColor.g, percent), 0.0f, 1.0f);
        colors[i].b = clamp(lerp(fromColor.b, toColor.b, percent), 0.0f, 1.0f);
        colors[i].a = clamp(lerp(fromColor.a, toColor.a, percent), 0.0f, 1.0f);
    }

    for (int i = 0; i < particleData.size(); ++i)
    {
        particleData[i].x = positions[i].X;
        particleData[i].y = positions[i].Y;
        particleData[i].z = positions[i].Z;
        particleData[i].scale = positions[i].W;
        particleData[i].color = colors[i];
    }

    // Update the instance buffer
    if (particleData.size() > 0)
    {
        sg_range data = {};
        data.ptr = &particleData[0];
        data.size = particleData.size() * sizeof(ParticleData);
        sg_update_buffer(bindings.vertex_buffers[1], &data);
    }
}
