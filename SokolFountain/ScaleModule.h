#pragma once
#include <random>
#include "Interfaces/IModule.h"

class ScaleModule : public IModule
{
	ScaleModule(float start, float endMin, float endMax);

public:
	void Tick(float deltaTime, std::vector<int32_t> inputs);

private:
	const uint8_t INPUT_X = 0;
	const uint8_t INPUT_Y = 1;
	const uint8_t INPUT_Z = 2;
	const uint8_t INPUT_SCALE = 3;
	
	const uint8_t INPUT_LIFEPERCENT = 4;
	const uint8_t INPUT_LIFESECONDS = 5;
	const uint8_t INPUT_VELX = 6;
	const uint8_t INPUT_VELY = 7;
	
	const uint8_t INPUT_R = 8;
	const uint8_t INPUT_G = 9;
	const uint8_t INPUT_B = 10;
	const uint8_t INPUT_A = 11;
	
	float startScale;
	float endScale;
};