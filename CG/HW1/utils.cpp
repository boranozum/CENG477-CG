#include "utils.h"

float dotProduct(parser::Vec3f vec1, parser::Vec3f vec2){

    float res = vec1.x*vec2.x + vec1.y* vec2.y + vec1.z*vec2.z;

    return res;
}

float findLength(parser::Vec3f vec){

    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

parser::Vec3f crossProduct(parser::Vec3f vec1, parser::Vec3f vec2){
    parser::Vec3f res;

    res.x = vec1.y*vec2.z - vec1.z*vec2.y;
    res.y = vec1.x*vec2.z - vec1.z*vec2.x;
    res.z = vec1.x*vec2.y - vec1.y*vec2.x;

    return res;
}

parser::Vec3f normalize(parser::Vec3f vec){

    float length = findLength(vec);

    parser::Vec3f res;

    res.x = vec.x/length;
    res.y = vec.y/length;
    res.z = vec.z/length;

    return res;
}

parser::Vec3f vecSum(parser::Vec3f vec1, float scalar1, parser::Vec3f vec2, float scalar2){

    parser::Vec3f lhs;
    parser::Vec3f rhs;

    lhs.x *= scalar1;
    lhs.y *= scalar1;
    lhs.z *= scalar1;

    rhs.x *= scalar2;
    rhs.y *= scalar2;
    rhs.z *= scalar2;

    parser::Vec3f res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;

    return res;
}

Ray spawnRay(int i, int j, parser::Camera camera){

    parser::Vec3f w;

    w.x = -camera.gaze.x;
    w.y = -camera.gaze.y;
    w.z = -camera.gaze.z;

    parser::Vec3f u = crossProduct(camera.up, w);

    u = normalize(u);
    
    parser::Vec3f m = vecSum(camera.position, 1, camera.gaze, camera.near_distance);

    parser::Vec3f q = vecSum(m,1,u,camera.near_plane.x);

    q = vecSum(q,1,camera.up,camera.near_plane.w);

    float s_u = (i+0.5)*(camera.near_plane.y-camera.near_plane.x)/camera.image_width;
    float s_v = (j+0.5)*(camera.near_plane.w-camera.near_plane.z)/camera.image_height;

    parser::Vec3f s = vecSum(q,1,u,s_u);

    s = vecSum(s,1,camera.up,-1*s_v);

    Ray ray;
    
    ray.origin = camera.position;
    ray.direction = normalize(vecSum(s,1,camera.position,-1));

    return ray;
}

float sphereIntersect(parser::Vec3f center, float radius, Ray ray){

    float B = 2 * dotProduct(ray.direction, vecSum(ray.origin,1,center,-1));
    float A = dotProduct(ray.direction,ray.direction);
    float C = dotProduct(vecSum(ray.origin,1,center,-1), vecSum(ray.origin,1,center,-1));

    float disc = B*B-4*A*C;

    if(disc < 0) return -1;

    float t_1 = ((-1*B) - sqrt(disc))/(2*A);
    float t_2 = ((-1*B) + sqrt(disc))/(2*A);

    float t_min = std::min(t_1,t_2);

    return t_min;
}
