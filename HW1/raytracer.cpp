#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "utils.h"

using namespace std;

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

        int i = 0;

        // Generating a ray for each pixel in the image
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Ray ray = spawnRay(x, y, scene.cameras[k]);

                // Checking whether the ray strikes with an object, assigns t with positive value for success or -1 for failure
                Strike strike = findStrike(ray, scene);

                // Color assignment based on strike success
                image[i++] = strike.pixel.x;
                image[i++] = strike.pixel.y;
                image[i++] = strike.pixel.z;
            }
        }

        // The resulting image is written into a ppm file
        write_ppm(scene.cameras[k].image_name.c_str(), image, width, height);

        // For convenient testing, WILL BE DELETED BEFORE SUBMISSION!!!
        string str = "display " + scene.cameras[k].image_name;
        system(str.c_str());
    }
}
