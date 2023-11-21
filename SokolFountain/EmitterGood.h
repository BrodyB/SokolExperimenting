#pragma once
#include <vector>
#include "HandmadeMath.h"
#include "ParticleTypes.h"
#include "sokol_gfx.h"
#include "Interfaces/IModule.h"
#include "HandmadeMath.h"

class EmitterGood
{
public: // Constructor & Methods
    EmitterGood(const char* imgPath, const std::vector<vertex_t>* vertices, const std::vector<uint16_t>* indices, float durationMin, float durationMax, int32_t maxParticles);
    void Start();
    void Stop(bool immediately = false);
    void Tick(float deltaTime, hmm_mat4 params);
    void SetOffsetPosition(float x, float y, float z);
    void AddModule(IModule& mod);

private:
    bool isActive;
    // Relative position from parent System
    float offsetPos[3] = { 0 };
    // Relative rotation from parent System
    float offsetRot[3] = { 0 };

    float lifespanMin;
    float lifespanMax;
    const int emissionRate = 40;
    int32_t maxParticles;
    int32_t indexCount;
    uint64_t oldTime;
    double emitTimes[32] = { 0 };
    double removalTimes[32] = { 0 };
    double positionTimes[32] = { 0 };
    double colorTimes[32] = { 0 };
    double updateTimes[32] = { 0 };
    int updateIndex = 0;
    double updateMax;

    sg_bindings bindings = { 0 };
    sg_pipeline pipeline = { 0 };
    std::vector<ParticleData> particleData;
    std::vector<hmm_vec4> positions;
    std::vector<color_t> colors;
    std::vector<hmm_vec2> lifetimes;
    color_t fromColor;
    color_t toColor;

    void EmitParticles(float deltaTime);
    void UpdateInstances(float deltaTime);
};
