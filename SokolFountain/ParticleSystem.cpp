#include "ParticleSystem.h"
#include <math.h>
#include "HandmadeMath.h"

#include "EmitterBad.h"
#include "ScaleModule.h"
#include "ColorModule.h"

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
    EmitterBad newEmitter("data/smoke.png", vertices, indices, 2.0f, 4.0f, 512);

    ScaleModule* scale = new ScaleModule(0.1f, 64.0f, 128.0f);
    newEmitter.AddModule(*scale);

    color_t start{ 0.0f, 1.0f, 0.0f, 1.0f };
    color_t end{ 1.0f, 0.0f, 0.0f, 0.0f };
    ColorModule* color = new ColorModule(start, end);
    newEmitter.AddModule(*color);

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
