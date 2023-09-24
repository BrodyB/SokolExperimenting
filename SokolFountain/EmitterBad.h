#pragma once
#include <vector>
#include "Interfaces/IEmitter.h"

class EmitterBad : public IEmitter
{
public: // Constructor & Methods
    EmitterBad(ParticleSystem& system, const vertex_t* vertices, const uint32_t* indices, float durationMin, float durationMax, int32_t maxParticles);
    void Start();
    void Stop(bool immediately = false);
    void Tick(float deltaTime, hmm_mat4* params) override;
    void SetOffsetPosition(float x, float y, float z) override;
    void SetOffsetRotation(float x, float y, float z) override;
    
private:
    
    struct Particle
    {
        // a float is 4 bytes
        float lifetime; // 0..1 percentage through particle life
        float seconds; // Seconds particle has been alive
        float maxDuration; // Seconds the particle will live total 
        float velX, velY;
        // this struct is 20 bytes: inside the 32 byte cache line
        // but won't be in alignment
    };

    struct ParticleData
    {
        float x, y, z, scale;
        float r, g, b, a;
    };

    std::vector<Particle> particles;
    std::vector<ParticleData> particleData;
};
