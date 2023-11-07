#pragma once
#include <vector>

#include "ParticleTypes.h"
#include "EmitterBad.h"

union hmm_mat4;

class ParticleSystem
{
public:
    float position[4];  // X, Y, Z, padding
    float rotation[4];  // X, Y, Z, padding

    ParticleSystem();
    
    void Tick(float deltaTime, hmm_mat4 params);
    void AddEmitter(std::vector<vertex_t>* vertices, std::vector<uint16_t>* indices);
    void Start();
    void Stop(bool immediately = false);

protected:
    std::vector<EmitterBad> emitters;

private:
    float time = 0.0f;
    float sway = 0.0f;
};
