#include "utils.h"
#include <iostream>

using namespace std;

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

float determinantSolver3x3(std::vector<std::vector<float>> matrix){
    float leftmat,rightmat,midmat;
    rightmat = determinantSolver2x2(matrix[1][1],matrix[1][2],matrix[2][1],matrix[2][2]);
    leftmat = determinantSolver2x2(matrix[1][0],matrix[1][1],matrix[2][0],matrix[2][1]);
    midmat = determinantSolver2x2(matrix[1][0],matrix[2][0],matrix[1][2],matrix[2][2]);
    return (matrix[0][0]*rightmat) - (matrix[0][1]*midmat) + (matrix[0][2] * leftmat) ;
}

float determinantSolver2x2(float a1, float a2, float b1, float b2){
    return (a1*b2) - (a2*b1);
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


float triangleIntersect(Ray ray, Vec3f a, Vec3f b, Vec3f c, Vec3f o, Vec3f d){

    vector<vector<float>> a_matrix = {{a.x-b.x,a.x-c.x,d.x},
                                      {a.y-b.y,a.y-c.y,d.y},
                                      {a.z-b.z,a.z-c.z,d.z}};

    vector<vector<float>> beta_matrix = {{a.x-o.x,a.x-c.x,d.x},
                                         {a.y-o.y,a.y-c.y,d.y},
                                         {a.z-o.z,a.z-c.z,d.z}};

    vector<vector<float>> gama_matrix = {{a.x-b.x,a.x-o.x,d.x},
                                         {a.y-b.y,a.y-o.y,d.y},
                                         {a.z-b.z,a.z-o.z,d.z}};
    
    vector<vector<float>> t_matrix = {{a.x-b.x,a.x-c.x,a.x-o.x},
                                      {a.y-b.y,a.y-c.y,a.y-o.y},
                                      {a.z-b.z,a.z-c.z,a.z-o.z}};

    float beta = determinantSolver3x3(beta_matrix)/determinantSolver3x3(a_matrix);
    float gama = determinantSolver3x3(gama_matrix)/determinantSolver3x3(a_matrix);
    float t = determinantSolver3x3(t_matrix)/determinantSolver3x3(a_matrix);
    
    if(beta + gama > 1){
        return -1;
    }

    if(beta < 0){
        return -1;
    }

    if(gama < 0){
        return -1;
    }

    return t;    
}

Strike findStrike(Ray ray, Scene scene){
    
    vector<Sphere> spheres = scene.spheres;
    vector<Triangle> triangles = scene.triangles;
    vector<Mesh> meshes = scene.meshes;
    
    Strike strike;
    strike.t = -1;
    
    for (size_t i = 0; i < spheres.size(); i++)
    {
        Vec3f sphere_center = scene.vertex_data[spheres[i].center_vertex_id-1];
        float radius = spheres[i].radius;
        float temp = sphereIntersect(sphere_center,radius,ray);

        if(strike.t == -1){
            strike.t = temp;
        }     
        else if(temp > 0 && strike.t > 0){
            strike.t = min(strike.t,temp);
        }
    }
    
    for (size_t i = 0; i < triangles.size(); i++){
        
        Vec3f a = scene.vertex_data[scene.triangles[i].indices.v0_id-1];
        Vec3f b = scene.vertex_data[scene.triangles[i].indices.v1_id-1];
        Vec3f c = scene.vertex_data[scene.triangles[i].indices.v2_id-1];
        
        float temp_t_of_tri = triangleIntersect(ray,a,b,c,ray.origin,ray.direction);
        if (strike.t == -1 ){
            strike.t = temp_t_of_tri;
        }
        else if(temp_t_of_tri > 0 && strike.t > 0){
            strike.t = min(strike.t,temp_t_of_tri);
        }
    }
    
    for (size_t i = 0; i < meshes.size(); i++){  
        for (size_t j = 0; j < meshes[i].faces.size(); j++){
            Vec3f a = scene.vertex_data[meshes[i].faces[j].v0_id-1];
            Vec3f b = scene.vertex_data[meshes[i].faces[j].v1_id-1];
            Vec3f c = scene.vertex_data[meshes[i].faces[j].v2_id-1];
        
            float temp_t_of_mesh = triangleIntersect(ray,a,b,c,ray.origin,ray.direction);
            if (strike.t == -1 ){
                strike.t = temp_t_of_mesh;
            }
            else if(temp_t_of_mesh > 0 && strike.t > 0){
                strike.t = min(strike.t,temp_t_of_mesh);
            }
        }
        
    }
    

    return strike;
}

