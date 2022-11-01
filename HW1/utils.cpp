#include "utils.h"
#include <iostream>

using namespace std;

float dotProduct(Vec3f vec1, Vec3f vec2){       // To find the dot product of two vectors

    float res = vec1.x*vec2.x + vec1.y* vec2.y + vec1.z*vec2.z;

    return res;
}

float findLength(Vec3f vec){        // To find a length of a vector

    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

Vec3f findSphereNormal(Vec3f center, Vec3f intersectionPoint){
    Vec3f res = vecSum(intersectionPoint,1,center,-1);
    res = normalize(res);

    return res;
}


Vec3f findTriangleNormal(Vec3f a, Vec3f b, Vec3f c){
    Vec3f res;
    
    Vec3f cmina = vecSum(c,1,a,-1);
    Vec3f bmina = vecSum(b,1,a,-1);
    
    res = crossProduct(bmina,cmina);
    res = normalize(res);
    
    return res;
}

Vec3f crossProduct(Vec3f vec1, Vec3f vec2){     // To find the cross product of two vectors
    
    Vec3f res;

    res.x = vec1.y*vec2.z - vec1.z*vec2.y;
    res.y = -(vec1.x*vec2.z - vec1.z*vec2.x);
    res.z = vec1.x*vec2.y - vec1.y*vec2.x;

    return res;
}

Vec3f normalize(Vec3f vec){           // To normalize a vector

    float length = findLength(vec);

    Vec3f res;

    res.x = vec.x/length;
    res.y = vec.y/length;
    res.z = vec.z/length;

    return res;
}

Vec3f vecSum(Vec3f vec1, float scalar1, Vec3f vec2, float scalar2){     // Vector summation with scalar values

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

Ray spawnRay(int i, int j, Camera camera){          // Ray generation

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

float determinantSolver3x3(std::vector<std::vector<float>> matrix){         // To solve 3x3 determinants
    
    float leftmat,rightmat,midmat;
    
    rightmat = determinantSolver2x2(matrix[1][1],matrix[1][2],matrix[2][1],matrix[2][2]);
    leftmat = determinantSolver2x2(matrix[1][0],matrix[1][1],matrix[2][0],matrix[2][1]);
    midmat = determinantSolver2x2(matrix[1][0],matrix[2][0],matrix[1][2],matrix[2][2]);
    
    float res = (matrix[0][0]*rightmat) - (matrix[0][1]*midmat) + (matrix[0][2] * leftmat);

    return res;
}

float determinantSolver2x2(float a1, float a2, float b1, float b2){        // Determinant helper
    return (a1*b2) - (a2*b1);
}
    

float sphereIntersect(Vec3f center, float radius, Ray ray){         // To implement sphere intersection formula

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


float triangleIntersect(Ray ray, Vec3f a, Vec3f b, Vec3f c, Vec3f o, Vec3f d){      // To implement triangular intersection formula

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

    // if(determinantSolver3x3(a_matrix) == 0){
    //     return -1;
    // }

    float beta = determinantSolver3x3(beta_matrix)/determinantSolver3x3(a_matrix);
    float gama = determinantSolver3x3(gama_matrix)/determinantSolver3x3(a_matrix);
    float t = determinantSolver3x3(t_matrix)/determinantSolver3x3(a_matrix);
    
    if((beta + gama > 1) || (beta < 0) || (gama < 0) || (t <= 0)){
        return -1;
    }

    return t;
}

Strike findStrike(Ray ray, Scene scene){           // To find intersections with spheres, triangles and meshes
    
    vector<Sphere> spheres = scene.spheres;
    vector<Triangle> triangles = scene.triangles;
    vector<Mesh> meshes = scene.meshes;
    
    Strike strike;
    strike.t = -1;
    strike.pixel = scene.background_color;

    for (size_t i = 0; i < spheres.size(); i++)
    {
        Vec3f sphere_center = scene.vertex_data[spheres[i].center_vertex_id-1];
        float radius = spheres[i].radius;
        float temp = sphereIntersect(sphere_center,radius,ray);

        if(temp > 0){
            if(strike.t < 0 || (strike.t > 0 && temp < strike.t)){
                
                strike.t = temp;
                strike.material = scene.materials[spheres[i].material_id-1];

                strike.intersectionPoint = findIntersectionPoint(ray, strike.t);

                Vec3f surfaceNormal = findSphereNormal(sphere_center,strike.intersectionPoint);    

                //cout << surfaceNormal.x << " " << surfaceNormal.y << " " << surfaceNormal.z << endl;
                
                Vec3f pixelColor = findAmbient(strike.material,scene.ambient_light);

                for (size_t i = 0; i < scene.point_lights.size(); i++)
                {
                    Vec3f irradiance = findIrradiance(strike.intersectionPoint,scene.point_lights[i]);
                    Vec3f diffuse = findDiffuse(irradiance,scene.point_lights[i], strike, surfaceNormal);
                    Vec3f specular = findSpecular(irradiance,scene.point_lights[i], strike, surfaceNormal, ray);

                    
                    pixelColor = vecSum(pixelColor,1,vecSum(diffuse,1,specular,1),1);
                }

                // TODO: CHECK IF BETWEEN 0 255

                strike.pixel.x = int(pixelColor.x+0.5);
                strike.pixel.y = int(pixelColor.y+0.5);            
                strike.pixel.z = int(pixelColor.z+0.5);  
                
            }     
        }
    }
    
    for (size_t i = 0; i < triangles.size(); i++){
        
        Vec3f a = scene.vertex_data[scene.triangles[i].indices.v0_id-1];
        Vec3f b = scene.vertex_data[scene.triangles[i].indices.v1_id-1];
        Vec3f c = scene.vertex_data[scene.triangles[i].indices.v2_id-1];

        float temp_t_of_tri = triangleIntersect(ray,a,b,c,ray.origin,ray.direction);

        if(temp_t_of_tri > 0){

            if (strike.t < 0 || (strike.t > 0 && temp_t_of_tri < strike.t)){
            
                strike.t = temp_t_of_tri;
            
                strike.material = scene.materials[triangles[i].material_id-1];
                
                strike.intersectionPoint = findIntersectionPoint(ray, strike.t);

                Vec3f surfaceNormal = findTriangleNormal(a,b,c);

                Vec3f pixelColor = findAmbient(strike.material,scene.ambient_light);

                for (size_t i = 0; i < scene.point_lights.size(); i++)
                {
                    Vec3f irradiance = findIrradiance(strike.intersectionPoint,scene.point_lights[i]);
                    Vec3f diffuse = findDiffuse(irradiance,scene.point_lights[i], strike, surfaceNormal);
                    Vec3f specular = findSpecular(irradiance,scene.point_lights[i], strike, surfaceNormal, ray);

                    pixelColor = vecSum(pixelColor,1,vecSum(diffuse,1,specular,1),1);
                }

                strike.pixel.x = int(pixelColor.x+0.5);
                strike.pixel.y = int(pixelColor.y+0.5);            
                strike.pixel.z = int(pixelColor.z+0.5);  
            }
            
        }
    
    }
    
    for (size_t i = 0; i < meshes.size(); i++){  
        for (size_t j = 0; j < meshes[i].faces.size(); j++){
            Vec3f a = scene.vertex_data[meshes[i].faces[j].v0_id-1];
            Vec3f b = scene.vertex_data[meshes[i].faces[j].v1_id-1];
            Vec3f c = scene.vertex_data[meshes[i].faces[j].v2_id-1];
        
            float temp_t_of_mesh = triangleIntersect(ray,a,b,c,ray.origin,ray.direction);
            if(temp_t_of_mesh > 0){
                if(strike.t < 0 || (strike.t > 0 && temp_t_of_mesh < strike.t)){
                    strike.t = temp_t_of_mesh;

                    strike.material = scene.materials[meshes[i].material_id-1];

                    strike.intersectionPoint = findIntersectionPoint(ray, strike.t);

                    Vec3f surfaceNormal = findTriangleNormal(a,b,c);

                    Vec3f pixelColor = findAmbient(strike.material,scene.ambient_light);


                    for (size_t i = 0; i < scene.point_lights.size(); i++){
                        Vec3f irradiance = findIrradiance(strike.intersectionPoint,scene.point_lights[i]);
                        Vec3f diffuse = findDiffuse(irradiance,scene.point_lights[i], strike, surfaceNormal);
                        Vec3f specular = findSpecular(irradiance,scene.point_lights[i], strike, surfaceNormal, ray);

                        pixelColor = vecSum(pixelColor,1,vecSum(diffuse,1,specular,1),1);
                    }

                    strike.pixel.x = int(pixelColor.x+0.5);
                    strike.pixel.y = int(pixelColor.y+0.5);            
                    strike.pixel.z = int(pixelColor.z+0.5);  
                }
            }
        }
    }
    
    return strike;
}

Vec3f findAmbient(Material material, Vec3f ambientLight){
    
    Vec3f result;

    result.x = material.ambient.x * ambientLight.x;
    result.y = material.ambient.y * ambientLight.y;
    result.z = material.ambient.z * ambientLight.z;

    return result;
}

Vec3f findIntersectionPoint(Ray ray, float t){
    return vecSum(ray.origin,1,ray.direction,t);
}

Vec3f findIrradiance(Vec3f intersectionPoint, PointLight pointLight){
    Vec3f res;
    float distance = findLength(vecSum(intersectionPoint,-1,pointLight.position,1));
    
    float distanceSquared = distance*distance; 

    if(distanceSquared != 0){
        res.x = pointLight.intensity.x/distanceSquared;
        res.y = pointLight.intensity.y/distanceSquared;
        res.z = pointLight.intensity.z/distanceSquared;
    }

    return res;    
}

Vec3f findDiffuse(Vec3f irradiance, PointLight pointLight, Strike strike, Vec3f surfaceNormal){
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

Vec3f findSpecular(Vec3f irradiance, PointLight pointLight, Strike strike, Vec3f surfaceNormal, Ray ray){

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

