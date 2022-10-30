#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "utils.h"

typedef unsigned char RGB[3];

int main(int argc, char *argv[])
{
    // Sample usage for reading an XML scene file
    parser::Scene scene;

    scene.loadFromXml(argv[1]);

    for (size_t k = 0; k < scene.cameras.size(); k++)
    {
        int width = scene.cameras[k].image_width, height = scene.cameras[k].image_height;

        unsigned char *image = new unsigned char[width * height * 3];

        int i = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Ray ray = spawnRay(x, y, scene.cameras[0]);

                Strike strike = findStrike(ray, scene);

                if (strike.t > 0)
                {
                    image[i++] = 255;
                    image[i++] = 255;
                    image[i++] = 255;
                }

                else
                {
                    image[i++] = scene.background_color.x;
                    image[i++] = scene.background_color.y;
                    image[i++] = scene.background_color.z;
                }
            }
        }

        write_ppm(scene.cameras[k].image_name.c_str(), image, width, height);
    }
}
