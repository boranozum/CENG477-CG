#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "utils.h"

typedef unsigned char RGB[3];

int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    parser::Scene scene;

    scene.loadFromXml(argv[1]);

    // The code below creates a test pattern and writes
    // it to a PPM file to demonstrate the usage of the
    // ppm_write function.
    //
    // Normally, you would be running your ray tracing
    // code here to produce the desired image.

    const RGB BAR_COLOR[8] =
    {
        { 255, 255, 255 },  // 100% White
        { 255, 255,   0 },  // Yellow
        {   0, 255, 255 },  // Cyan
        {   0, 255,   0 },  // Green
        { 255,   0, 255 },  // Magenta
        { 255,   0,   0 },  // Red
        {   0,   0, 255 },  // Blue
        {   0,   0,   0 },  // Black
    };

    int width = scene.cameras[0].image_width, height = scene.cameras[0].image_height;
    int columnWidth = width / 8;

    unsigned char* image = new unsigned char [width * height * 3];

    int i = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Ray ray = spawnRay(x,y,scene.cameras[0]);

            float t = sphereIntersect(scene.vertex_data[scene.spheres[0].center_vertex_id-1], scene.spheres[0].radius, ray);


            if(t > 0){
                image[i++] = 255;
                image[i++] = 255;
                image[i++] = 255;
            }            

            else{
                image[i++] = 0;
                image[i++] = 0;
                image[i++] = 0;
            }
        }
    }


    write_ppm("test.ppm", image, width, height);

}
