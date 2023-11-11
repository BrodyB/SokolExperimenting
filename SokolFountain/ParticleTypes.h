#pragma once
// Struct for a sprite vertex
struct vertex_t
{
	float x, y, z, scale;
    float u, v;
};

struct color_t
{
    color_t() : r(1), g(1), b(1), a(1) {}
    color_t(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    ~color_t() {}
    float r, g, b, a;
};


// Struct for info
struct ParticleInstance
{
    // a float is 4 bytes
    float x = 0, y = 0, z = 0, scale = 1.0f;
    color_t color;
    float seconds; // Seconds particle has been alive
    float maxDuration; // Seconds the particle will live total
    float velX = 0, velY = 0;
    // this struct is 48 bytes: inside the 64 byte cache line

    float lifetime() const { return seconds / maxDuration; } // 0..1 percentage through particle life
};

// Struct for data sent to the instance vertex buffer
struct ParticleData
{
    float x, y, z, scale;
    color_t color;
};