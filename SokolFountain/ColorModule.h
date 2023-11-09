#pragma once
#include "Interfaces/IModule.h"

class ColorModule : public IModule
{
public:
	ColorModule(color_t start, color_t end);

	// Inherited via IModule
	virtual void Tick(float deltaTime, ParticleInstance* instance) override;

private:
	color_t startColor;
	color_t endColor;

	color_t lerp_color(color_t a, color_t b, float t);
};

