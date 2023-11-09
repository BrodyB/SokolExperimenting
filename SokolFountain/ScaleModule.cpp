#include "ScaleModule.h"
#include "Utility.h"

ScaleModule::ScaleModule(float start, float endMin, float endMax)
{
    startScale = start;
    endScale = random(endMin, endMax);
}

void ScaleModule::Tick(float deltaTime, ParticleInstance* instance)
{
    instance->scale = lerp(startScale, endScale, instance->lifetime());
}
