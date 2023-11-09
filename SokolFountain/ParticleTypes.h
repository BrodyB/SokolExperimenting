#pragma once
// Struct for a sprite vertex
typedef struct
{
	float x, y, z, scale;
} vertex_t;


// Struct for info
struct ParticleInstance
{
    // a float is 4 bytes
    float x, y, z;
    float scale;
    float seconds; // Seconds particle has been alive
    float maxDuration; // Seconds the particle will live total 
    float lifetime() const { return seconds / maxDuration; } // 0..1 percentage through particle life
    float velX, velY;
    // this struct is 20 bytes: inside the 32 byte cache line
    // but won't be in alignment
};

// Struct for data sent to the instance vertex buffer
struct ParticleData
{
    float x, y, z, scale;
    float r, g, b, a;
};