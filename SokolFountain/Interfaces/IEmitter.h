#pragma once

#include <vector>

class ParticleSystem;

union hmm_mat4;
struct sg_bindings;
struct sg_pipeline;

class IEmitter
{
public:
#pragma region Public Methods
    virtual void Tick(float deltaTime, hmm_mat4* params) = 0;
    virtual void SetOffsetPosition(float x, float y, float z) = 0;
    virtual void SetOffsetRotation(float x, float y, float z) = 0;
#pragma endregion

protected:
    float* parentPos;
    float* parentRot;
    // Relative position from parent System
    float offsetPos[3] = { 0 };
    // Relative rotation from parent System
    float offsetRot[3] = { 0 };

    float lifespanMin;
    float lifespanMax;

    int32_t indexCount;
    sg_bindings* bindings;
    sg_pipeline* pipeline;

    // All effects applied to particle instances
    // std::vector<IModule> modules;
};
