#ifndef _UTILS_H
#define _UTILS_H

#include <math.h>
#include "parser.h"
#include <vector>

using namespace parser;

typedef struct Ray{
    Vec3f origin;
    Vec3f direction;
} Ray;

typedef struct Strike{
    float t;
    Vec3f surfaceNormal;
    Material material;
    Vec3f intersectionPoint;
    Vec3i pixel;
} Strike;

float dotProduct( Vec3f const &vec1,  Vec3f const &vec2);

float findLength( Vec3f const& vec);

Vec3f crossProduct( Vec3f const &vec1,  Vec3f const &vec2);

Vec3f normalize( Vec3f const &vec);

Vec3f vecSum( Vec3f const &vec1, float scalar1, Vec3f const &vec2, float scalar2);

Ray spawnRay(int i, int j,  Camera const &camera);

float determinantSolver3x3(float matrix[3][3]);

float determinantSolver2x2(float a1, float a2, float b1, float b2);

float sphereIntersect( Vec3f const &center, float radius, Ray &ray);

float triangleIntersect( Ray const &ray,  Vec3f const &a,  Vec3f const &b,  Vec3f const &c,  Vec3f const &o,  Vec3f const &d, float epsilon);

Vec3f findIrradiance( Vec3f const &intersectionPoint,  PointLight const &pointLight);

Vec3f findIntersectionPoint( Ray const &ray, float t);

Vec3f findAmbient( Material const &material,  Vec3f const &ambientLight);

Strike findStrike( Ray const &ray,  Scene const &scene,int recursionCount);

bool findShadowStrike( Ray const &ray,  Scene const &scene,float light_t);

Vec3f findDiffuse( Vec3f const &irradiance,  PointLight const &pointLight,  Strike const &strike,  Vec3f const &surfaceNormal);

Vec3f findSpecular( Vec3f const &irradiance,  PointLight const &pointLight,  Strike const &strike,  Vec3f const &surfaceNormal,  Ray const &ray);

Vec3f findSphereNormal( Vec3f const &center,  Vec3f const &intersectionPoint);

Vec3f findTriangleNormal( Vec3f const &a,  Vec3f const &b,  Vec3f const &c);

Ray findReflectionRay( Ray const &ray,  Vec3f const &surfaceNormal,  Vec3f const &intersectionPoint,  Scene const &scene);

#endif
