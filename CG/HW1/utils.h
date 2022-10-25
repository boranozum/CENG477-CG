#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include "parser.h"

typedef struct Ray{
    parser::Vec3f origin;
    parser::Vec3f direction;
} Ray;

float dotProduct(parser::Vec3f vec1, parser::Vec3f vec2);

float findLength(parser::Vec3f vec);

parser::Vec3f crossProduct(parser::Vec3f vec1, parser::Vec3f vec2);

parser::Vec3f normalize(parser::Vec3f vec);

parser::Vec3f vecSum(parser::Vec3f vec1, float scalar1, parser::Vec3f vec2, float scalar2);

Ray spawnRay(int i, int j, parser::Camera camera);

float sphereIntersect(parser::Sphere sphere, Ray ray);

#endif
