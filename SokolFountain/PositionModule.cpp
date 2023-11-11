#include "PositionModule.h"

PositionModule::PositionModule(float velX, float velY)
{
	velocityX = velX;
	velocityY = velY;
}

void PositionModule::Tick(float deltaTime, ParticleInstance* instance)
{
	instance->velX += velocityX;
	instance->velY += velocityY;
}
