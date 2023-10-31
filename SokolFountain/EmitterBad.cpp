#include "EmitterBad.h"

#include "sokol_gfx.h"
#include "HandmadeMath.h"
// #include "ParticleSystem.h"
#include "ParticleTypes.h"
#include "textured.glsl.h"
#include "Utility.h"

EmitterBad::EmitterBad(const std::vector<vertex_t>* vertices, const std::vector<uint32_t>* indices, float durationMin, float durationMax, int32_t maxParticles)
{
    // parentPos = system->position;
    // parentRot = system->rotation;

    lifespanMin = durationMin;
    lifespanMax = durationMax;
    particleData.resize(maxParticles);

    // Create the vertex buffer
    sg_buffer_desc vbuffer = { 0 };
    vbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
    vbuffer.data = SG_RANGE(*vertices);
    vbuffer.label = "emitter vertices";
    bindings.vertex_buffers[0] = sg_make_buffer(&vbuffer);

    // Create the index buffer
    sg_buffer_desc ibuffer = { 0 };
    ibuffer.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuffer.data = SG_RANGE(*indices);
    ibuffer.label = "emitter indices";
    bindings.index_buffer = sg_make_buffer(&ibuffer);
    indexCount = static_cast<int32_t>(indices->size());

    // Dynamic buffer for instance data
    sg_buffer_desc instbuffer = { 0 };
    instbuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
    instbuffer.size = maxParticles * sizeof(Particle);
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
    pip.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_pos].buffer_index = 0;
    pip.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_color0].buffer_index = 0;
	
    // Instance data buffer
    pip.layout.attrs[ATTR_vs_inst_pos].format = SG_VERTEXFORMAT_FLOAT4;
    pip.layout.attrs[ATTR_vs_inst_pos].buffer_index = 1;
    pip.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    pipeline = sg_make_pipeline(&pip);
}

void EmitterBad::Start()
{
    for (int i = 0; i < static_cast<int>(particleData.size()); ++i)
    {
        particleData[i].x = random(-500.0f, 500.0f);
        particleData[i].y = random(-500.0f, 500.0f);
        particleData[i].z = random(-5.0f, 5.0f);
        particleData[i].scale = random(0.25f, 1.25f);
        
        particleData[i].r = random(0.0f, 1.0f);
        particleData[i].g = random(0.0f, 1.0f);
        particleData[i].b = random(0.0f, 1.0f);
        particleData[i].a = random(0.5f, 1.0f);
    }
}

void EmitterBad::Stop(bool immediately)
{
    
}

void EmitterBad::Tick(float deltaTime, hmm_mat4& params)
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
