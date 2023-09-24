#include <random>

inline float random(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr(min, max);
    return static_cast<float>(distr(gen));
}
