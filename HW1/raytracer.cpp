#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "utils.h"
#include "pthread.h"

using namespace std;

typedef struct ThreadArgs{
    int y;
    int y_size;
    Camera camera;
    Scene scene;
    unsigned char* image;
    int i_value;
} ThreadArgs;

pthread_t threads[8];

void* threadRoutine(void* args){

    auto args1 = (ThreadArgs*) args;

    int y_val = args1->y;
    int y_size = args1->y_size;

    Camera camera = args1->camera;
    Scene scene = args1->scene;

    int i_value = args1->i_value;
    int width = camera.image_width;

    // Created a matrix that represents the image with each cell corresponds to a pixel
    unsigned char *image = args1->image;

    // Generating a ray for each pixel in the image
    for (int y = y_val; y < y_val+y_size; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Ray ray = spawnRay(x, y, camera);

            // Checking whether the ray strikes with an object, assigns t with positive value for success or -1 for failure
            Strike strike = findStrike(ray, scene,0);

            // Color assignment based on strike success
            // MODIFY THIS
            image[i_value++] = min(max(strike.pixel.x,0),255);
            image[i_value++] = min(max(strike.pixel.y,0),255);
            image[i_value++] = min(max(strike.pixel.z,0),255);
        }
    }

    pthread_exit(nullptr);
}

int main(int argc, char *argv[])
{
    parser::Scene scene;

    // Scene file is loaded into the scene variable
    scene.loadFromXml(argv[1]);

    // If the scene contains multiple cameras, iterate through each one
    for (size_t k = 0; k < scene.cameras.size(); k++)
    {

        // Extract the resolution
        int width = scene.cameras[k].image_width, height = scene.cameras[k].image_height;

        // Created a matrix that represents the image with each cell corresponds to a pixel
        unsigned char *image = new unsigned char[width * height * 3];


        for (int i = 0; i < 8; ++i) {
            ThreadArgs* threadArgs = new ThreadArgs;
            threadArgs->image = image;
            threadArgs->scene = scene;
            threadArgs->camera = scene.cameras[k];
            threadArgs->y = i*height/8;
            threadArgs->y_size = height/8;
            threadArgs->i_value = i*(height/8)*width*3;

            pthread_create(&threads[i], nullptr,threadRoutine,threadArgs);
        }

        for (int i = 0; i < 8; ++i) {
            pthread_join(threads[i], nullptr);
        }


        // The resulting image is written into a ppm file
        write_ppm(scene.cameras[k].image_name.c_str(), image, width, height);
    }
}
