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
    
    void Tick(float deltaTime, hmm_mat4 params) const;
    void AddEmitter(std::vector<vertex_t>* vertices, std::vector<uint32_t>* indices);
    void Start() const;
    void Stop(bool immediately = false) const;

protected:
    std::vector<EmitterBad> emitters;
};
