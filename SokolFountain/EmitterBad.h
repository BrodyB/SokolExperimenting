#pragma once

#include <vector>
#include "HandmadeMath.h"
#include "ParticleTypes.h"
#include "sokol_gfx.h"

class EmitterBad
{
public: // Constructor & Methods
    EmitterBad(const std::vector<vertex_t>* vertices, const std::vector<uint16_t>* indices, float durationMin, float durationMax, int32_t maxParticles);
    void Start();
    void Stop(bool immediately = false);
    void Tick(float deltaTime, hmm_mat4 params);
    void SetOffsetPosition(float x, float y, float z);
    void SetOffsetRotation(float x, float y, float z);
    
private:
    struct ParticleInstance
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

    // float* parentPos;
    // float* parentRot;
    // Relative position from parent System
    float offsetPos[3] = { 0 };
    // Relative rotation from parent System
    float offsetRot[3] = { 0 };

    float lifespanMin;
    float lifespanMax;
    const float emissionRate = 0.1f;
    float emissionTimer = 0.0;
    int32_t maxParticles;
    int32_t indexCount;

    sg_bindings bindings = { 0 };
    sg_pipeline pipeline = { 0 };
    std::vector<ParticleData> particleData;
    std::vector<ParticleInstance> particleInstances;

    void EmitParticles(float deltaTime);
    void UpdateInstances(float deltaTime);
};
