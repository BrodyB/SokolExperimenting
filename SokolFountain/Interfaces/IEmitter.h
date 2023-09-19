#pragma once

#include <vector>
#include "IModule.h"
#include "../particleTypes.h"

class IEmitter
{
public:
#pragma region Public Methods
    virtual void Tick(float deltaTime) = 0;
    virtual void SetOffsetPosition(float x, float y, float z) = 0;
    virtual void SetOffsetRotation(float x, float y, float z) = 0;
#pragma endregion

protected:
    // Relative position/rotation from parent System
    std::vector<float> offsetPos;
    std::vector<float> offsetRot;
    
    // Vertices/indices used for the base particle geometry
    vertex_t vertices[4];
    uint16_t indices[6];

    // All effects applied to particle instances
    std::vector<IModule> modules;
};
