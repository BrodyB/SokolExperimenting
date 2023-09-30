#pragma once
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

// Struct for a sprite vertex
typedef struct
{
	float x, y, z, scale;
	float u, v;
} vertex_t;