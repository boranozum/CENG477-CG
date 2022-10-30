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
    Material material;
    Vec3f intersectionPoint;
    Vec3i pixel;
} Strike;

float dotProduct(Vec3f vec1, Vec3f vec2);

float findLength(Vec3f vec);

Vec3f crossProduct(Vec3f vec1, Vec3f vec2);

Vec3f normalize(Vec3f vec);

Vec3f vecSum(Vec3f vec1, float scalar1, Vec3f vec2, float scalar2);

Ray spawnRay(int i, int j, Camera camera);

float determinantSolver3x3(std::vector<std::vector<float>> matrix);

float determinantSolver2x2(float a1, float a2, float b1, float b2);

float sphereIntersect(Vec3f center, float radius, Ray ray);

float triangleIntersect(Ray ray, Vec3f a, Vec3f b, Vec3f c, Vec3f o, Vec3f d);

Vec3f findIrradiance(Vec3f intersectionPoint, PointLight pointLight);

Vec3f findIntersectionPoint(Ray ray, float t);

Vec3f findAmbient(Material material, Vec3f ambientLight);

Strike findStrike(Ray ray, Scene scene);

Vec3f findDiffuse(Vec3f irradiance, PointLight pointLight, Strike strike, Vec3f surfaceNormal);

Vec3f findSpecular(Vec3f irradiance, PointLight pointLight, Strike strike, Vec3f surfaceNormal, Ray ray);

Vec3f findSphereNormal(Vec3f center, Vec3f intersectionPoint ,float radius);

Vec3f findTriangleNormal(Vec3f a, Vec3f b, Vec3f c);

#endif
