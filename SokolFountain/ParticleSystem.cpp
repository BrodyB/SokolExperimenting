#include "ParticleSystem.h"
#include <math.h>
#include "HandmadeMath.h"

#include "EmitterBad.h"
#include "PositionModule.h"
#include "ScaleModule.h"
#include "ColorModule.h"

ParticleSystem::ParticleSystem() = default;

void ParticleSystem::Tick(float deltaTime, hmm_mat4 params)
{
    time += deltaTime;

    for (EmitterBad& emitter : emitters)
    {
        sway = sinf(time) * 100.0f;
        emitter.SetOffsetPosition(sway, -200.0f, 0.0f);
        emitter.Tick(deltaTime, params);
    }
}

void ParticleSystem::AddEmitter(std::vector<vertex_t>* vertices, std::vector<uint16_t>* indices)
{
    EmitterBad newEmitter("data/smoke.png", vertices, indices, 6.0f, 8.0f, 100000);

    PositionModule* pos = new PositionModule(0.1f, 1.0f);
    newEmitter.AddModule(*pos);

    ScaleModule* scale = new ScaleModule(64.0f, 256.0f, 360.0f);
    newEmitter.AddModule(*scale);

    color_t start{ 0.8f, 0.8f, 0.8f, 1.0f };
    color_t end{ 0.3f, 0.3f, 0.3f, 0.0f };
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
