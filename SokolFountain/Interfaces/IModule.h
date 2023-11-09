#pragma once
#include "../ParticleTypes.h"

class IModule
{
public:
	virtual void Tick(float deltaTime, ParticleInstance* instance) = 0;
};