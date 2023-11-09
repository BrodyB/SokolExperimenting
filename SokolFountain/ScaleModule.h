#pragma once
#include <random>
#include "ParticleTypes.h"
#include "Interfaces/IModule.h"

class ScaleModule : public IModule
{
public:
	ScaleModule(float start, float endMin, float endMax);
	void Tick(float deltaTime, ParticleInstance* instance);

private:
	float startScale;
	float endScale;
};