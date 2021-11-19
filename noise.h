#ifndef NOISE_H
#define NOISE_H

typedef struct {
    float x, y;
} vector2;

float interpolate(float, float, float);

vector2 randomGradient(int, int);

float dotGridGradient(int, int, float, float);

float perlin(float, float);

#endif
