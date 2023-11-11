#pragma once
#include "Interfaces/IModule.h"

class PositionModule : public IModule
{
public:
	PositionModule(float velX, float velY);

	// Inherited via IModule
	virtual void Tick(float deltaTime, ParticleInstance* instance);

private:
	float velocityX;
	float velocityY;
};

