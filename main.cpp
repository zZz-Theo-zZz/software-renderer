#include "tgaimage.h"
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
    int startx = x0;
    int starty = y0;
    int endx = x1;
    int endy = y1;

    bool steepCurve = false;

    if (std::abs(x1 - x0) < std::abs(y1 - y0))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steepCurve = true;
    }

    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int deltax = x1 - x0;
    int deltay = y1 - y0;
    int initialError = std::abs(2 * deltay);
    int error = 0;
    int y = y0;

    int increment = 1;
    if (y1 <= y0)
        increment = -1;

    for (int x = x0; x <= x1; ++x)
    {
        if (steepCurve)
            image.set(y, x, color);
        else
            image.set(x, y, color);

        error += initialError;
        if (error > deltax)
        {
            error -= 2 * deltax;
            y += increment;
        }
    }

    image.set(startx, starty, green);
    image.set(endx, endy, red);
}

int main(int argc, char** argv)
{
    TGAImage image(100, 100, TGAImage::RGB);
    image.set(0, 0, blue);
    image.set(99, 99, blue);

    line(50, 50, 90, 50, image, {255, 128, 0, 255}); // orange
    line(50, 50, 90, 70, image, {255, 128, 0, 255}); // orange
    line(50, 50, 90, 10, image, {102, 0, 204, 255}); // purple
    line(90, 10, 50, 50, image, {204, 0, 102, 255 }); // purple
    line(50, 50, 90, 90, image, {255, 255, 0, 255}); // yellow

    line(50, 50, 10, 90, image, {255, 153, 255, 255}); // pink
    line(50, 50, 10, 10, image, white);


    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}