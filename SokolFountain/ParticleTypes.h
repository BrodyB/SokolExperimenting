#pragma once
#include <HandmadeMath.h>

// Good Particles with Bad Alignment
struct Particle
{
	// a float is 4 bytes
	float lifetime;
	float x, y, z, scale;
	float velX, velY;
	float r, g, b, a;
	// this struct is 44 bytes: well outside the 32 byte cache line
	// so this is gonna blow out our cache, like, every time
};


struct ParticleData
{
	hmm_vec4 pos[4096];
	hmm_vec2 lifetime[4096];
	hmm_vec2 velocity[4096];
	hmm_vec4 color[4096];
};

// Struct for a sprite vertex
typedef struct
{
	float x, y, z;
	float u, v;
} vertex_t;