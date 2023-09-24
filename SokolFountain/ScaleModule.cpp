#include "ScaleModule.h"

ScaleModule::ScaleModule(float start, float endMin, float endMax)
{
    startScale = start;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr((double)endMin, (double)endMax);
    
    endScale = static_cast<float>(distr(gen));
}

void ScaleModule::Tick(float deltaTime, std::vector<int32_t> inputs)
{
    
    // Inputs:
        // POS xyz scale (float4)
        // LIFETIME (0..1, seconds) (float2)
        // VELOCITY xy (float2)
        // COLOR rgba (float4)
}
