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
	a.r = lerp(a.r, b.r, t);
	a.g = lerp(a.g, b.g, t);
	a.b = lerp(a.b, b.b, t);
	a.a = lerp(a.a, b.a, t);

	return a;
}
