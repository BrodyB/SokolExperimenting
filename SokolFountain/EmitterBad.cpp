﻿#include "EmitterBad.h"

#include "sokol_gfx.h"
#include "HandmadeMath.h"
#include "ParticleTypes.h"
#include "textured.glsl.h"
#include "../libs/stb/stb_image.h"
#include "Utility.h"

EmitterBad::EmitterBad(const char* imgPath, const std::vector<vertex_t>* vertices, const std::vector<uint16_t>* indices, float durationMin, float durationMax, int32_t maxParticles)
{
    lifespanMin = durationMin;
    lifespanMax = durationMax;
    this->maxParticles = maxParticles;

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
    vbuffer.data = sg_range{vertices->data(), (int32_t)vertices->size() * sizeof(vertex_t)};
    vbuffer.label = "emitter vertices";
    bindings.vertex_buffers[0] = sg_make_buffer(&vbuffer);

    // Create the index buffer
    sg_buffer_desc ibuffer = { 0 };
    ibuffer.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuffer.data = sg_range{ indices->data(), (int32_t)indices->size() * sizeof(uint16_t)}; // problem?
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

void EmitterBad::Start()
{
    isActive = true;
}

void EmitterBad::Stop(bool immediately)
{
    isActive = false;
}

void EmitterBad::Tick(float deltaTime, hmm_mat4 params)
{
    EmitParticles(deltaTime);
    UpdateInstances(deltaTime);

    // Draw emitter particles
    sg_apply_pipeline(pipeline);
    sg_apply_bindings(bindings);
    vs_params_t vs_params;
    vs_params.mvp = params;
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    sg_draw(0, indexCount, (int)particleData.size()); // Base element, Number of elements, instances
}

void EmitterBad::SetOffsetPosition(float x, float y, float z)
{
    offsetPos[0] = x;
    offsetPos[1] = y;
    offsetPos[2] = z;
}

void EmitterBad::SetOffsetRotation(float x, float y, float z)
{
    offsetRot[0] = x;
    offsetRot[1] = y;
    offsetRot[2] = z;
}

void EmitterBad::AddModule(IModule& mod)
{
    modules.push_back(&mod);
}

void EmitterBad::EmitParticles(float deltaTime)
{
    if (!isActive) return;
    emissionTimer += deltaTime;

    /*char buffer[50];
    sprintf_s(buffer, "Timer: %f // Delta: %f", emissionTimer, deltaTime);
    LOG(buffer);*/

    if (emissionTimer >= emissionRate)
    {
        emissionTimer -= emissionRate;

        int size = (int)particleData.size();
        int cap = (int)particleData.capacity();

        if (particleData.size() < maxParticles)
        {
            ParticleData data;
            particleData.insert(particleData.begin(), data);

            ParticleInstance inst;
            inst.x = offsetPos[0] + random(-100.0f, 100.0f);
            inst.y = offsetPos[1] + random(-100.0f, 100.0f);
            inst.z = 0.0f;
            inst.maxDuration = random(lifespanMin, lifespanMax);
            inst.seconds = 0.0f;
            particleInstances.insert(particleInstances.begin(), inst);
        }
    }
}

void EmitterBad::UpdateInstances(float deltaTime)
{
    for (int i = 0; i < particleData.size(); ++i)
    {
        particleInstances[i].seconds += deltaTime;

        for (IModule* module : modules)
        {
            module->Tick(deltaTime, &particleInstances[i]);
        }

        particleInstances[i].x += particleInstances[i].velX * deltaTime;
        particleInstances[i].y += particleInstances[i].velY * deltaTime;

        // Push updated values to the data struct
        particleData[i].x = particleInstances[i].x;
        particleData[i].y = particleInstances[i].y;
        particleData[i].z = particleInstances[i].z;
        particleData[i].scale = particleInstances[i].scale;

        particleData[i].color = particleInstances[i].color;
    }

    // Update the instance buffer
    if (particleData.size() > 0)
    {
        // Remove expired instances at the back of the list
        ParticleInstance oldInst = particleInstances.back();
        if (oldInst.seconds > oldInst.maxDuration)
        {
            particleInstances.pop_back();
            particleData.pop_back();
        }

        sg_range data = {};
        data.ptr = &particleData[0];
        data.size = particleData.size() * sizeof(ParticleData);
        sg_update_buffer(bindings.vertex_buffers[1], &data);
    }
}
