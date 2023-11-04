#include "ParticleSystem.h"
#include "HandmadeMath.h"

#include "EmitterBad.h"

ParticleSystem::ParticleSystem()
= default;

void ParticleSystem::Tick(float deltaTime, hmm_mat4 params) const
{
    for (EmitterBad emitter : emitters)
    {
        emitter.Tick(deltaTime, params);
    }
}

void ParticleSystem::AddEmitter(std::vector<vertex_t>* vertices, std::vector<uint16_t>* indices)
{
    emitters.emplace_back(vertices, indices, 2.0f, 4.0f, 512);
}

void ParticleSystem::Start() const
{
    for (EmitterBad emitter : emitters)
    {
        emitter.Start();
    }
}

void ParticleSystem::Stop(bool immediately) const
{
    for (EmitterBad emitter : emitters)
    {
        emitter.Stop();
    }
}
