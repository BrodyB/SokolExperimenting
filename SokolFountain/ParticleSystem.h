#pragma once
#include <vector>

#include "EmitterBad.h"
#include "Interfaces/IEmitter.h"

class ParticleSystem
{
public:
    float position[4];  // X, Y, Z, padding
    float rotation[4];  // X, Y, Z, padding

    ParticleSystem();
    
    void Tick(float deltaTime, hmm_mat4* params);
    void AddEmitter(vertex_t vertices[], uint32_t indices[]);
    void Start();
    void Stop(bool immediately = false);

protected:
    std::vector<EmitterBad> emitters;
};
