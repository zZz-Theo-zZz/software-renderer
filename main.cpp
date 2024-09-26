#include "tgaimage.h"
#include "model.h"
#include <iostream>
#include <cmath>

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

void boundingbox(Vec2i t0, Vec2i t1, Vec2i t2, Vec2i windowSize, Vec2i& min, Vec2i& max)
{
    min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    max = { 0, 0 };

    min = { std::min(t0.x, min.x), std::min(t0.y, min.y) };
    min = { std::min(t1.x, min.x), std::min(t1.y, min.y) };
    min = { std::min(t2.x, min.x), std::min(t2.y, min.y) };

    max = { std::max(t0.x, max.x), std::max(t0.y, max.y) };
    max = { std::max(t1.x, max.x), std::max(t1.y, max.y) };
    max = { std::max(t2.x, max.x), std::max(t2.y, max.y) };

    min = { std::max(min.x, 0), std::max(min.y, 0) };
    max = { std::min(max.x, windowSize.x), std::min(max.y, windowSize.y) };
}

int dot(const Vec2i& a, const Vec2i& b)
{
    return a.x * b.x + a.y * b.y;
}

int crossProduct(const Vec2i& a, const Vec2i& b)
{
    return a.x * b.y - b.x * a.y;
}

bool insideTriangle(Vec2i point, Vec2i t[3])
{
    bool a = crossProduct(point - t[2], t[1] - t[2]) <= 0;
    bool b = crossProduct(point - t[1], t[0] - t[1]) <= 0;
    bool c = crossProduct(point - t[0], t[2] - t[0]) <= 0;
    
    return a == b && b == c;
}

void triangle2(Vec2i triangle[3], TGAImage& image, TGAColor color)
{
    Vec2i min, max;
    boundingbox(triangle[0], triangle[1], triangle[2], {image.get_width(), image.get_height()}, min, max);

    for (int x = min.x; x <= max.x; ++x)
    {
        for (int y = min.y; y <= max.y; ++y)
        {
            if (insideTriangle({x, y}, triangle))
                image.set(x, y, color);
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
    line(t0, t1, image, { 122, 25, 200, 255 });
    line(t1, t2, image, { 122, 25, 200, 255 });
    line(t2, t0, image, { 122, 25, 200, 255 });

    if (t0.y > t1.y)
        std::swap(t0, t1);

    if (t0.y > t2.y)
        std::swap(t0, t2);

    if (t1.y > t2.y)
        std::swap(t1, t2);    

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
    int windowWidth = 2000;
    int windowHeight = 2000;

    TGAImage image(windowWidth, windowHeight, TGAImage::RGB);

    Model model("african_head.obj");

    Vec3f lightDirection = { 0.f, 0.f, -1};

    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++) {
            Vec3f vertex = model.vert(face[j]);
            screen_coords[j] = Vec2i((vertex.x + 1.) * windowWidth / 2., (vertex.y + 1.) * windowHeight/ 2.);
            world_coords[j] = vertex;
        }

        Vec3f normal = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        normal.normalize();

        float lightIntensity = normal * lightDirection;
        if (lightIntensity > 0.0f)
            triangle2(screen_coords, image, TGAColor(255 * lightIntensity, 255 * lightIntensity, 255 * lightIntensity, 255));
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}