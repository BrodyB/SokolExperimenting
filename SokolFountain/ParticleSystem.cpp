#include "ParticleSystem.h"
#include <math.h>
#include "HandmadeMath.h"

#include "EmitterBad.h"

ParticleSystem::ParticleSystem()
= default;

void ParticleSystem::Tick(float deltaTime, hmm_mat4 params)
{
    time += deltaTime;

    for (EmitterBad& emitter : emitters)
    {
        sway = sinf(time) * 400.0f;
        emitter.SetOffsetPosition(sway, 0.0f, 0.0f);
        emitter.Tick(deltaTime, params);
    }
}

void ParticleSystem::AddEmitter(std::vector<vertex_t>* vertices, std::vector<uint16_t>* indices)
{
    EmitterBad newEmitter(vertices, indices, 2.0f, 4.0f, 512);
    emitters.push_back(newEmitter);
}

void ParticleSystem::Start()
{
    for (EmitterBad& emitter : emitters)
    {
        emitter.Start();
    }
}

void ParticleSystem::Stop(bool immediately)
{
    for (EmitterBad& emitter : emitters)
    {
        emitter.Stop();
    }
}
