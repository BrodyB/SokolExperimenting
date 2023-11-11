#pragma once
// Struct for a sprite vertex
typedef struct
{
	float x, y, z, scale;
    float u, v;
} vertex_t;

typedef struct
{
    float r, g, b, a;
} color_t;


// Struct for info
struct ParticleInstance
{
    // a float is 4 bytes
    float x, y, z, scale;
    color_t color;
    float seconds; // Seconds particle has been alive
    float maxDuration; // Seconds the particle will live total
    float velX, velY;
    // this struct is 48 bytes: inside the 64 byte cache line

    float lifetime() const { return seconds / maxDuration; } // 0..1 percentage through particle life
};

// Struct for data sent to the instance vertex buffer
struct ParticleData
{
    float x, y, z, scale;
    color_t color;
};