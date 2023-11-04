#include "EmitterBad.h"

#include "sokol_gfx.h"
#include "HandmadeMath.h"
// #include "ParticleSystem.h"
#include "ParticleTypes.h"
#include "textured.glsl.h"
#include "Utility.h"

EmitterBad::EmitterBad(const std::vector<vertex_t>* vertices, const std::vector<uint16_t>* indices, float durationMin, float durationMax, int32_t maxParticles)
{
    // parentPos = system->position;
    // parentRot = system->rotation;

    lifespanMin = durationMin;
    lifespanMax = durationMax;
    particleData.resize(maxParticles);

    bindings = { 0 };
    pipeline = { 0 };

    // Create the vertex buffer
    sg_buffer_desc vbuffer = { 0 };
    vbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
    vbuffer.data = sg_range{ vertices->data(), vertices->size() * sizeof(vertex_t)};
    vbuffer.label = "emitter vertices";
    bindings.vertex_buffers[0] = sg_make_buffer(&vbuffer);

    // Create the index buffer
    sg_buffer_desc ibuffer = { 0 };
    ibuffer.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuffer.data = sg_range{ indices->data(), indices->size() * sizeof(uint16_t)}; // problem?
    ibuffer.label = "emitter indices";
    bindings.index_buffer = sg_make_buffer(&ibuffer);
    indexCount = static_cast<int32_t>(indices->size());

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
    // Vertex buffer
    pip.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3;
    pip.layout.attrs[ATTR_vs_pos].buffer_index = 0;
    //pip.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
    //pip.layout.attrs[ATTR_vs_color0].buffer_index = 0;
	
    // Instance data buffer
    pip.layout.attrs[ATTR_vs_inst_pos].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_inst_pos].buffer_index = 1;
    pip.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    pipeline = sg_make_pipeline(&pip);
}

void EmitterBad::Start()
{
    const int count = static_cast<int>(particleData.size());

    for (int i = 0; i < count; ++i)
    {
        particleData[i].x = random(-500.0f, 500.0f);
        particleData[i].y = random(-500.0f, 500.0f);
        particleData[i].z = random(4.5f, 5.0f);
        particleData[i].scale = random(0.85f, 1.25f);
        
        //particleData[i].r = random(0.0f, 1.0f);
        //particleData[i].g = random(0.0f, 1.0f);
        //particleData[i].b = random(0.0f, 1.0f);
        //particleData[i].a = random(0.5f, 1.0f);
    }

    // Update the instance data
    sg_range data = {};
    data.ptr = &particleData[0];
    data.size = (size_t)count * sizeof(ParticleData);
    sg_update_buffer(bindings.vertex_buffers[1], &data);
}

void EmitterBad::Stop(bool immediately)
{
    
}

void EmitterBad::Tick(float deltaTime, hmm_mat4 params)
{
    /*
    for (int i = 0; i < static_cast<int>(particles.size()); ++i)
    {
        Particle* particle = &particles[i];

        particle->seconds += deltaTime;
        particle->lifetime = particle->seconds / particle->maxDuration;
        
        if (particle->lifetime > 1.0f)
            particle->lifetime = 1.0f;
        
        if (particle->seconds >= particle->maxDuration)
        {
            particles.erase(particles.begin() + i);
            continue;
        }
    }
    */

    // Draw emitter particles
    sg_apply_pipeline(pipeline);
    sg_apply_bindings(bindings);
    vs_params_t vs_params;
    vs_params.mvp = params;
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    sg_draw(0, indexCount, static_cast<int>(particleData.size())); // Base element, Number of elements, instances
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
