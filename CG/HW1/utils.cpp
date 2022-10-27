#include "utils.h"
#include <iostream>

float dotProduct(Vec3f vec1, Vec3f vec2){

    float res = vec1.x*vec2.x + vec1.y* vec2.y + vec1.z*vec2.z;

    return res;
}

float findLength(Vec3f vec){

    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

Vec3f crossProduct(Vec3f vec1, Vec3f vec2){
    Vec3f res;

    res.x = vec1.y*vec2.z - vec1.z*vec2.y;
    res.y = -(vec1.x*vec2.z - vec1.z*vec2.x);
    res.z = vec1.x*vec2.y - vec1.y*vec2.x;

    return res;
}

Vec3f normalize(Vec3f vec){

    float length = findLength(vec);

    Vec3f res;

    res.x = vec.x/length;
    res.y = vec.y/length;
    res.z = vec.z/length;

    return res;
}

Vec3f vecSum(Vec3f vec1, float scalar1, Vec3f vec2, float scalar2){

    vec1.x *= scalar1;
    vec1.y *= scalar1;
    vec1.z *= scalar1;

    vec2.x *= scalar2;
    vec2.y *= scalar2;
    vec2.z *= scalar2;

    Vec3f res;

    res.x = vec1.x + vec2.x;
    res.y = vec1.y + vec2.y;
    res.z = vec1.z + vec2.z;

    return res;
}

Ray spawnRay(int i, int j, Camera camera){

    Vec3f w;

    w.x = -1*camera.gaze.x;
    w.y = -1*camera.gaze.y;
    w.z = -1*camera.gaze.z;

    Vec3f u = crossProduct(camera.up, w);

    u = normalize(u);
    
    Vec3f m = vecSum(camera.position, 1, camera.gaze, camera.near_distance);

    Vec3f q = vecSum(m,1,u,camera.near_plane.x);

    q = vecSum(q,1,camera.up,camera.near_plane.w);

    float s_u = (i+0.5)*(camera.near_plane.y-camera.near_plane.x)/camera.image_width;
    float s_v = (j+0.5)*(camera.near_plane.w-camera.near_plane.z)/camera.image_height;

    Vec3f s = vecSum(q,1,u,s_u);

    s = vecSum(s,1,camera.up,-1*s_v);

    Ray ray;
    
    ray.origin = camera.position;
    ray.direction = vecSum(s,1,camera.position,-1);

    return ray;
}

float sphereIntersect(Vec3f center, float radius, Ray ray){

    float B = 2 * dotProduct(ray.direction, vecSum(ray.origin,1,center,-1));
    float A = dotProduct(ray.direction,ray.direction);
    float C = dotProduct(vecSum(ray.origin,1,center,-1), vecSum(ray.origin,1,center,-1))- (radius*radius);

    float disc = B*B-4*A*C;

    if(disc < 0) return -1;

    float t_1 = ((-1*B) - sqrt(disc))/(2*A);
    float t_2 = ((-1*B) + sqrt(disc))/(2*A);

    float t_min = std::min(t_1,t_2);

    return t_min;
}
