#include "ParticleSystem.h"

#include "EmitterBad.h"

ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::Tick(float deltaTime, hmm_mat4* params)
{
    for (EmitterBad emitter : emitters)
    {
        emitter.Tick(deltaTime, params);
    }
}

void ParticleSystem::AddEmitter(vertex_t vertices[], uint32_t indices[])
{
    // emitters.push_back(new EmitterBad(this, vertices, indices, 2.0f, 4.0f, 512));
}

void ParticleSystem::Start()
{
    /*
    for (EmitterBad emitter : emitters)
    {
        emitter.Start();
    }
    */
}

void ParticleSystem::Stop(bool immediately)
{
    /*
    for (EmitterBad emitter : emitters)
    {
        emitter.Stop();
    }
    */
}
