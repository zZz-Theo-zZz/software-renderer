#include "tgaimage.h"
#include "model.h"
#include <iostream>
#include <limits>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
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
}

void line(Vec2i a, Vec2i b, TGAImage& image, TGAColor color)
{
    line(a.x, a.y, b.x, b.y, image, color);
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
    line(t0, t1, image, { 122, 25, 200, 255 });
    line(t1, t2, image, { 122, 25, 200, 255 });
    line(t2, t0, image, { 122, 25, 200, 255 });

    // sorting vertices by ascending y
    if (t0.y > t1.y)
        std::swap(t0, t1);

    if (t0.y > t2.y)
        std::swap(t0, t2);

    if (t1.y > t2.y)
        std::swap(t1, t2);    

    // fill bottom and upper triangle with line drawing algorithm
    // first triangle is t1, t2, t3
    // we go up the line t1,t2 and t3, t2
    float invSlope1 = (t2.x - t1.x) / float(t2.y - t1.y);
    float invSlope2 = (t2.x - t0.x) / float(t2.y - t0.y);

    float edge1 = t2.x;
    float edge2 = t2.x;

    int y;
    for (y = t2.y; y > t1.y; --y)
    {
        line({ (int)std::round(edge1), y }, { (int)std::round(edge2), y }, image, color);

        edge1 -= invSlope1;
        edge2 -= invSlope2;
    }

    float invSlope3 = (t1.x - t0.x) / float(t1.y - t0.y);
    for (int y = t1.y; y >= t0.y; --y)
    {
        line({ (int)std::round(edge1), y }, { (int)std::round(edge2), y }, image, color);

        edge1 -= invSlope3;
        edge2 -= invSlope2;
    }
}

int main(int argc, char** argv)
{
    int windowWidth = 200;
    int windowHeight = 200;

    TGAImage image(windowWidth, windowHeight, TGAImage::RGB);

    //Model model("african_head.obj");

    //for (int i = 0; i < model.nfaces(); i++) {
    //    std::vector<int> face = model.face(i);
    //    for (int j = 0; j < 3; j++) {
    //        Vec3f v0 = model.vert(face[j]);
    //        Vec3f v1 = model.vert(face[(j + 1) % 3]);
    //        int x0 = (v0.x + 1.) * windowWidth / 2.;
    //        int y0 = (v0.y + 1.) * windowHeight / 2.;
    //        int x1 = (v1.x + 1.) * windowWidth / 2.;
    //        int y1 = (v1.y + 1.) * windowHeight / 2.;
    //        line(x0, y0, x1, y1, image, white);
    //    }
    //}

    Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
    Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
    Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}