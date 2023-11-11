#include "ColorModule.h"
#include "Utility.h"

ColorModule::ColorModule(color_t start, color_t end)
{
	startColor = start;
	endColor = end;
}

void ColorModule::Tick(float deltaTime, ParticleInstance* instance)
{
	instance->color = lerp_color(startColor, endColor, instance->lifetime());
}

color_t ColorModule::lerp_color(color_t a, color_t b, float t)
{
	a.r = clamp(lerp(a.r, b.r, t), 0.0f, 1.0f);
	a.g = clamp(lerp(a.g, b.g, t), 0.0f, 1.0f);
	a.b = clamp(lerp(a.b, b.b, t), 0.0f, 1.0f);
	a.a = clamp(lerp(a.a, b.a, t), 0.0f, 1.0f);

	return a;
}
