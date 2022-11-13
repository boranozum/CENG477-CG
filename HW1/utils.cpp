#include "utils.h"
#include <iostream>

using namespace std;

float dotProduct(Vec3f const  &vec1, Vec3f const  &vec2){       // To find the dot product of two vectors

    float res = (vec1.x)*(vec2.x) + (vec1.y)* (vec2.y) + (vec1.z)*(vec2.z);

    return res;
}

float findLength(Vec3f const & vec){        // To find a length of a vector

    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

Vec3f findSphereNormal(Vec3f const  &center, Vec3f const  &intersectionPoint){
    Vec3f res = vecSum(intersectionPoint,1,center,-1);
    res = normalize(res);

    return res;
}


Vec3f findTriangleNormal(Vec3f const  &a, Vec3f const  &b, Vec3f const  &c){
    Vec3f res;
    
    Vec3f cmina = vecSum(c,1,a,-1);
    Vec3f bminc = vecSum(b,1,c,-1);
    
    res = crossProduct(bminc,cmina);
    res = normalize(res);
    
    return res;
}

Vec3f crossProduct(Vec3f const  &vec1, Vec3f const  &vec2){     // To find the cross product of two vectors
    
    Vec3f res;

    res.x = vec1.y*vec2.z - vec1.z*vec2.y;
    res.y = -(vec1.x*vec2.z - vec1.z*vec2.x);
    res.z = vec1.x*vec2.y - vec1.y*vec2.x;

    return res;
}

Vec3f normalize(Vec3f const  &vec){           // To normalize a vector

    float length = findLength(vec);

    Vec3f res;

    res.x = vec.x/length;
    res.y = vec.y/length;
    res.z = vec.z/length;

    return res;
}

Vec3f vecSum(Vec3f const  &vec1, float scalar1, Vec3f const  &vec2, float scalar2){     // Vector summation with scalar values
    Vec3f res;

    res.x = vec1.x*scalar1 + vec2.x*scalar2;
    res.y = vec1.y*scalar1 + vec2.y*scalar2;
    res.z = vec1.z*scalar1 + vec2.z*scalar2;

    return res;
}

Ray spawnRay(int i, int j, Camera const  &camera){          // Ray generation

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

    ray.direction = normalize(ray.direction);

    return ray;
}

float determinantSolver3x3(float matrix[3][3]){         // To solve 3x3 determinants
    
    float leftmat,rightmat,midmat;
    
    rightmat = determinantSolver2x2(matrix[1][1],matrix[1][2],matrix[2][1],matrix[2][2]);
    leftmat = determinantSolver2x2(matrix[1][0],matrix[1][1],matrix[2][0],matrix[2][1]);
    midmat = determinantSolver2x2(matrix[1][0],matrix[2][0],matrix[1][2],matrix[2][2]);
    
    float res = (matrix[0][0]*rightmat) - (matrix[0][1]*midmat) + (matrix[0][2] * leftmat);

    return res;
}

float determinantSolver2x2(float a1, float a2, float  b1, float  b2){        // Determinant helper
    return (a1*b2) - (a2*b1);
}
    

float sphereIntersect(Vec3f const  &center, float  radius, Ray const   &ray){         // To implement sphere intersection formula

    Vec3f para = vecSum(ray.origin,1,center,-1);
    float B = 2 * dotProduct(ray.direction, para);
    float A = dotProduct(ray.direction,ray.direction);
    float C = dotProduct(para, para) - (radius*radius);

    float disc = B*B-4*A*C;

    if(disc < 0) return -1;

    float t_1 = ((-1*B) - sqrt(disc))/(2*A);
    float t_2 = ((-1*B) + sqrt(disc))/(2*A);

    float t_min = std::min(t_1,t_2);

    return t_min;
}


float triangleIntersect(Ray const  &ray, Vec3f const  &a, Vec3f const  &b, Vec3f const  &c, Vec3f const  &o, Vec3f const  &d, float epsilon){      // To implement triangular intersection formula

    float a_matrix[][3] = {{a.x-b.x,a.x-c.x,d.x},
                                      {a.y-b.y,a.y-c.y,d.y},
                                      {a.z-b.z,a.z-c.z,d.z}};

    float beta_matrix [][3] = {{a.x-o.x,a.x-c.x,d.x},
                                         {a.y-o.y,a.y-c.y,d.y},
                                         {a.z-o.z,a.z-c.z,d.z}};

    float gama_matrix[][3] = {{a.x-b.x,a.x-o.x,d.x},
                                         {a.y-b.y,a.y-o.y,d.y},
                                         {a.z-b.z,a.z-o.z,d.z}};
    
    float t_matrix[][3] = {{a.x-b.x,a.x-c.x,a.x-o.x},
                                      {a.y-b.y,a.y-c.y,a.y-o.y},
                                      {a.z-b.z,a.z-c.z,a.z-o.z}};

    if(determinantSolver3x3(a_matrix) == 0) return -1;


    float beta = determinantSolver3x3(beta_matrix)/determinantSolver3x3(a_matrix);
    float gama = determinantSolver3x3(gama_matrix)/determinantSolver3x3(a_matrix);
    float t = determinantSolver3x3(t_matrix)/determinantSolver3x3(a_matrix);
    
    if((beta + gama > 1) || (beta < 0) || (gama < 0)  || (gama > 1) || (t < 0) || (t < epsilon)){
        return -1;
    }

    return t;
}


Strike findStrike(Ray const  &ray, Scene const  &scene, int recursionCount){  // To find intersections with spheres, triangles and meshes

    vector<Sphere> spheres = scene.spheres;
    vector<Triangle> triangles = scene.triangles;
    vector<Mesh> meshes = scene.meshes;
    
    Strike strike;
    strike.t = -1;
    strike.pixel.x = scene.background_color.x;
    strike.pixel.y = scene.background_color.y;
    strike.pixel.z = scene.background_color.z;

    bool hit_tri = false, hit_sphere = false, hit_mesh = false;
    int sphere_index, tri_index, mesh_index,face_index;
    int sphere_size = spheres.size();

    for (int i = 0; i < sphere_size; i++)
    {
        Vec3f sphere_center = scene.vertex_data[spheres[i].center_vertex_id-1];
        float radius = spheres[i].radius;
        float temp = sphereIntersect(sphere_center,radius,ray);

        if(temp > 0){

            if(strike.t < 0 || (strike.t > 0 && temp < strike.t)){
                hit_sphere = true;
                hit_tri = false;
                hit_mesh = false;
                sphere_index = i;
                strike.t = temp;

            }     
        }
    }

    int triangle_size = triangles.size();
    for (int i = 0; i < triangle_size; i++){
        
        Vec3f a = scene.vertex_data[scene.triangles[i].indices.v0_id-1];
        Vec3f b = scene.vertex_data[scene.triangles[i].indices.v1_id-1];
        Vec3f c = scene.vertex_data[scene.triangles[i].indices.v2_id-1];

        float temp_t_of_tri = triangleIntersect(ray,a,b,c,ray.origin,ray.direction, scene.shadow_ray_epsilon);

        if(temp_t_of_tri > 0){

            if (strike.t < 0 || (strike.t > 0 && temp_t_of_tri < strike.t)){
                hit_tri = true;
                hit_sphere = false;
                hit_mesh = false;
                tri_index = i;
                strike.t = temp_t_of_tri;
            }
            
        }
    
    }
    int mesh_size = meshes.size();
    for (int i = 0; i < mesh_size; i++){
        int face_size = meshes[i].faces.size();
        for (size_t j = 0; j < face_size; j++){
            Vec3f a = scene.vertex_data[meshes[i].faces[j].v0_id-1];
            Vec3f b = scene.vertex_data[meshes[i].faces[j].v1_id-1];
            Vec3f c = scene.vertex_data[meshes[i].faces[j].v2_id-1];
        
            float temp_t_of_mesh = triangleIntersect(ray,a,b,c,ray.origin,ray.direction, scene.shadow_ray_epsilon);
            if(temp_t_of_mesh > 0){
                if(strike.t < 0 || (strike.t > 0 && temp_t_of_mesh < strike.t)){
                    hit_mesh = true;
                    hit_tri = false;
                    hit_sphere = false;
                    mesh_index = i;
                    face_index = j;
                    strike.t = temp_t_of_mesh;


                }
            }
        }
    }

    if(strike.t < 0) return strike;

    if(hit_sphere){
        Vec3f sphere_center = scene.vertex_data[spheres[sphere_index].center_vertex_id-1];

        strike.material = scene.materials[spheres[sphere_index].material_id-1];

        strike.intersectionPoint = findIntersectionPoint(ray, strike.t);

        strike.surfaceNormal = findSphereNormal(sphere_center,strike.intersectionPoint);
    }
    else if (hit_tri){
        Vec3f a = scene.vertex_data[scene.triangles[tri_index].indices.v0_id-1];
        Vec3f b = scene.vertex_data[scene.triangles[tri_index].indices.v1_id-1];
        Vec3f c = scene.vertex_data[scene.triangles[tri_index].indices.v2_id-1];

        strike.material = scene.materials[triangles[tri_index].material_id-1];

        strike.intersectionPoint = findIntersectionPoint(ray, strike.t);

        strike.surfaceNormal = findTriangleNormal(a,b,c);
    }

    else{
        Vec3f a = scene.vertex_data[meshes[mesh_index].faces[face_index].v0_id-1];
        Vec3f b = scene.vertex_data[meshes[mesh_index].faces[face_index].v1_id-1];
        Vec3f c = scene.vertex_data[meshes[mesh_index].faces[face_index].v2_id-1];

        strike.material = scene.materials[meshes[mesh_index].material_id-1];

        strike.intersectionPoint = findIntersectionPoint(ray, strike.t);

        strike.surfaceNormal = findTriangleNormal(a,b,c);
    }
    Vec3f pixelColor = findAmbient(strike.material,scene.ambient_light);

    int point_light_size = scene.point_lights.size();
    for (size_t i = 0; i < point_light_size; i++){
        Ray shadowRay;

        shadowRay.origin = vecSum(strike.intersectionPoint,1,strike.surfaceNormal,scene.shadow_ray_epsilon);
        shadowRay.direction = vecSum(scene.point_lights[i].position,1,shadowRay.origin,-1);
        shadowRay.direction = normalize(shadowRay.direction);

        float light_t = (scene.point_lights[i].position.x - shadowRay.origin.x) / shadowRay.direction.x;

        if(!findShadowStrike(shadowRay,scene, light_t)) {
            Vec3f irradiance = findIrradiance(strike.intersectionPoint, scene.point_lights[i]);
            Vec3f diffuse = findDiffuse(irradiance, scene.point_lights[i], strike, strike.surfaceNormal);
            Vec3f specular = findSpecular(irradiance, scene.point_lights[i], strike, strike.surfaceNormal,ray);
            Vec3f tempvec = vecSum(diffuse, 1, specular, 1);
            pixelColor = vecSum(pixelColor, 1,tempvec, 1);
        }
    }
    if (strike.material.is_mirror && recursionCount < scene.max_recursion_depth){

        Ray reflection_ray = findReflectionRay(ray,strike.surfaceNormal,strike.intersectionPoint,scene);
        Strike reflectedStrike = findStrike(reflection_ray,scene,recursionCount+1);
        pixelColor.x += reflectedStrike.pixel.x * strike.material.mirror.x;
        pixelColor.y += reflectedStrike.pixel.y * strike.material.mirror.y;
        pixelColor.z += reflectedStrike.pixel.z * strike.material.mirror.z;
    }

    strike.pixel.x = int(pixelColor.x+0.5);
    strike.pixel.y = int(pixelColor.y+0.5);
    strike.pixel.z = int(pixelColor.z+0.5);

    return strike;
}

bool findShadowStrike(Ray const  &ray, Scene const  &scene, float light_t){           // To find intersections with spheres, triangles and meshes

    vector<Sphere> spheres = scene.spheres;
    vector<Triangle> triangles = scene.triangles;
    vector<Mesh> meshes = scene.meshes;

    int sphere_size = spheres.size();
    for (size_t i = 0; i < sphere_size; i++)
    {
        Vec3f sphere_center = scene.vertex_data[spheres[i].center_vertex_id-1];
        float radius = spheres[i].radius;
        float temp = sphereIntersect(sphere_center,radius,ray);

        if(temp > 0 && light_t > temp) return true;
    }

    int triangle_size = triangles.size();
    for (size_t i = 0; i < triangle_size; i++){

        Vec3f a = scene.vertex_data[scene.triangles[i].indices.v0_id-1];
        Vec3f b = scene.vertex_data[scene.triangles[i].indices.v1_id-1];
        Vec3f c = scene.vertex_data[scene.triangles[i].indices.v2_id-1];

        float temp_t_of_tri = triangleIntersect(ray,a,b,c,ray.origin,ray.direction, scene.shadow_ray_epsilon);

        if(temp_t_of_tri > 0 && light_t > temp_t_of_tri) return true;
    }

    int mesh_size = meshes.size();
    for (size_t i = 0; i < mesh_size; i++){
        int face_size = meshes[i].faces.size();
        for (size_t j = 0; j < face_size; j++){
            Vec3f a = scene.vertex_data[meshes[i].faces[j].v0_id-1];
            Vec3f b = scene.vertex_data[meshes[i].faces[j].v1_id-1];
            Vec3f c = scene.vertex_data[meshes[i].faces[j].v2_id-1];

            float temp_t_of_mesh = triangleIntersect(ray,a,b,c,ray.origin,ray.direction, scene.shadow_ray_epsilon);

            if (temp_t_of_mesh > 0 && light_t > temp_t_of_mesh) return true;

        }
    }
    return false;
}

Vec3f findAmbient(Material const  &material, Vec3f const  &ambientLight){
    
    Vec3f result;

    result.x = material.ambient.x * ambientLight.x;
    result.y = material.ambient.y * ambientLight.y;
    result.z = material.ambient.z * ambientLight.z;

    return result;
}

Vec3f findIntersectionPoint(Ray const  &ray, float t){
    return vecSum(ray.origin,1,ray.direction,t);
}

Vec3f findIrradiance(Vec3f const  &intersectionPoint, PointLight const  &pointLight){
    Vec3f res;
    Vec3f temp = vecSum(intersectionPoint,-1,pointLight.position,1);
    float distance = findLength(temp);
    
    float distanceSquared = distance*distance; 

    if(distanceSquared != 0){
        res.x = pointLight.intensity.x/distanceSquared;
        res.y = pointLight.intensity.y/distanceSquared;
        res.z = pointLight.intensity.z/distanceSquared;
    }

    return res;    
}

Vec3f findDiffuse(Vec3f const  &irradiance, PointLight const  &pointLight, Strike const  &strike, Vec3f const  &surfaceNormal){
    Vec3f L = vecSum(pointLight.position,1,strike.intersectionPoint,-1);

    L = normalize(L);


    float nl = dotProduct(surfaceNormal,L) >= 0 ? dotProduct(surfaceNormal,L) : 0;

    
    Vec3f coeff = strike.material.diffuse;
    Vec3f res;

    res.x = (coeff.x)*(irradiance.x*nl);
    res.y = (coeff.y)*(irradiance.y*nl);
    res.z = (coeff.z)*(irradiance.z*nl);

    return res;
}

Vec3f findSpecular(Vec3f const  &irradiance, PointLight const  &pointLight, Strike const  &strike, Vec3f const  &surfaceNormal, Ray const  &ray){

    Vec3f res;
    Vec3f L = vecSum(pointLight.position,1,strike.intersectionPoint,-1);
    L = normalize(L);

    Vec3f halfVec = vecSum(ray.direction, -1, L, 1);
    halfVec = normalize(halfVec);

    float hn = dotProduct(surfaceNormal,halfVec) >= 0 ? dotProduct(surfaceNormal,halfVec) : 0;
    hn = pow(hn,strike.material.phong_exponent);
    Vec3f coeff = strike.material.specular; 

    res.x = (coeff.x)*(irradiance.x*hn);
    res.y = (coeff.y)*(irradiance.y*hn);
    res.z = (coeff.z)*(irradiance.z*hn);

    return res;
}

Ray findReflectionRay(Ray const  &ray, Vec3f const  &surfaceNormal,Vec3f const  &intersectionPoint, Scene const  &scene){
    Vec3f res;
    Vec3f negated_ray_direction;

    negated_ray_direction.x = -ray.direction.x;
    negated_ray_direction.y = -ray.direction.y;
    negated_ray_direction.z = -ray.direction.z;

    float angle_n_ray = dotProduct(negated_ray_direction,surfaceNormal);
    res = vecSum(ray.direction,1,surfaceNormal,2*angle_n_ray);
    res = normalize(res);

    Ray resultray;
    resultray.direction = res;
    resultray.origin = vecSum(intersectionPoint,1,surfaceNormal,scene.shadow_ray_epsilon);

    return resultray;
}


