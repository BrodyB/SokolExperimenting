#pragma once

#include <vector>
#include "HandmadeMath.h"
#include "ParticleTypes.h"
#include "sokol_gfx.h"
#include "Interfaces/IModule.h"

class EmitterBad
{
public: // Constructor & Methods
    EmitterBad(const char* imgPath, const std::vector<vertex_t>* vertices, const std::vector<uint16_t>* indices, float durationMin, float durationMax, int32_t maxParticles);
    void Start();
    void Stop(bool immediately = false);
    void Tick(float deltaTime, hmm_mat4 params);
    void SetOffsetPosition(float x, float y, float z);
    void SetOffsetRotation(float x, float y, float z);
    void AddModule(IModule& mod);
    
private:
    bool isActive;
    // Relative position from parent System
    float offsetPos[3] = { 0 };
    // Relative rotation from parent System
    float offsetRot[3] = { 0 };

    float lifespanMin;
    float lifespanMax;
    const float emissionRate = 0.01f;
    float emissionTimer = 0.0;
    int32_t maxParticles;
    int32_t indexCount;

    sg_bindings bindings = { 0 };
    sg_pipeline pipeline = { 0 };
    std::vector<ParticleData> particleData;
    std::vector<ParticleInstance> particleInstances;
    std::vector<IModule*> modules;

    void EmitParticles(float deltaTime);
    void UpdateInstances(float deltaTime);
};
