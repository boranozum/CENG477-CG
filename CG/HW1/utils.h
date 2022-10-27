#ifndef _UTILS_H
#define _UTILS_H

#include <math.h>
#include "parser.h"

using namespace parser;

typedef struct Ray{
    Vec3f origin;
    Vec3f direction;
} Ray;

float dotProduct(Vec3f vec1, Vec3f vec2);

float findLength(Vec3f vec);

Vec3f crossProduct(Vec3f vec1, Vec3f vec2);

Vec3f normalize(Vec3f vec);

Vec3f vecSum(Vec3f vec1, float scalar1, Vec3f vec2, float scalar2);

Ray spawnRay(int i, int j, Camera camera);

float sphereIntersect(Vec3f center, float radius, Ray ray);

#endif
