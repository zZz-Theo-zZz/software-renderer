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

void boundingbox(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i windowSize, Vec2i& min, Vec2i& max)
{
    min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    max = { 0, 0 };

    min = { (int)std::min<float>(t0.x, min.x), (int)std::min<float>(t0.y, min.y) };
    min = { (int)std::min<float>(t1.x, min.x), (int)std::min<float>(t1.y, min.y) };
    min = { (int)std::min<float>(t2.x, min.x), (int)std::min<float>(t2.y, min.y) };

    max = { (int)std::max<float>(t0.x, max.x), (int)std::max<float>(t0.y, max.y) };
    max = { (int)std::max<float>(t1.x, max.x), (int)std::max<float>(t1.y, max.y) };
    max = { (int)std::max<float>(t2.x, max.x), (int)std::max<float>(t2.y, max.y) };

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

void triangle(Vec3f vertices[3], Vec2f uv[3], float* depthBuffer, TGAImage& diffuseMap, float light, TGAImage& output)
{
    Vec2i min, max;
    Vec3f a = vertices[0];
    Vec3f b = vertices[1];
    Vec3f c = vertices[2];

    int width = output.get_width();
    int height = output.get_height();

    float diffuseHeight = diffuseMap.get_height();
    float diffuseWidth = diffuseMap.get_width();

    boundingbox(a, b, c, { width, height }, min, max);

    if (c.y == a.y)
        std::swap(a, b);

    float alphaDenominator = ((b.y - a.y) * (c.x - a.x) - (b.x - a.x) * (c.y - a.y));
    float betaDenominator = (c.y - a.y);

    if (std::isnormal(alphaDenominator))
    {
        for (int x = min.x; x <= max.x; ++x)
        {
            for (int y = min.y; y <= max.y; ++y)
            {
                float alpha = (a.x * (c.y - a.y) + (y - a.y) * (c.x - a.x) - x * (c.y - a.y)) / alphaDenominator;
                float beta = (y - a.y - alpha * (b.y - a.y)) / betaDenominator;
                float sigma = 1.0f - alpha - beta;

                if (sigma >= 0.0f && alpha >= 0.0f && beta >= 0.0f)
                {
                    float depth = a.z * sigma + alpha * b.z + beta * c.z;

                    if (depthBuffer[y * width + x] < depth)
                    {
                        depthBuffer[y * width + x] = depth;

                        Vec2f uvA = { uv[0].x * diffuseWidth, (1.0f - uv[0].y) * diffuseHeight };
                        Vec2f uvB = { uv[1].x * diffuseWidth, (1.0f - uv[1].y) * diffuseHeight };
                        Vec2f uvC = { uv[2].x * diffuseWidth, (1.0f - uv[2].y) * diffuseHeight };

                        Vec2f currentPixelUV = uvA * sigma + uvB * alpha + uvC * beta;

                        TGAColor sampledColor = diffuseMap.get(currentPixelUV.x, currentPixelUV.y);

                        depth = (depth + 1.0f) * 0.5f;

                        output.set(x, y, sampledColor * light);
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    const int windowWidth = 2000;
    const int windowHeight = 2000;

    float* depthBuffer = new float[windowWidth * windowHeight];
    for (int i = 0; i < windowHeight * windowWidth; ++i)
        depthBuffer[i] = -std::numeric_limits<float>::max();

    TGAImage image(windowWidth, windowHeight, TGAImage::RGB);

    Model model("african_head.obj");
    TGAImage diffuseMap;
    if (!diffuseMap.read_tga_file("african_head_diffuse.tga"))
    {
        std::cerr << "Couldn't read diffuse map" << std::endl;
        return 1;
    }

    Vec3f lightDirection = { 0.f, 0.f, -1};

    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<VertexInfo> face = model.face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec2f uv[3];

        for (int j = 0; j < 3; j++) {
            Vec3f vertex = model.vert(face[j].VertexId);
            uv[j] = model.uv(face[j].TexCoordId);

            screen_coords[j] = Vec3f((vertex.x + 1.) * windowWidth / 2., (vertex.y + 1.) * windowHeight/ 2., vertex.z);
            world_coords[j] = vertex;
        }

        Vec3f normal = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        normal.normalize();

        float lightIntensity = normal * lightDirection;
        if (lightIntensity > 0.0f)
            triangle(screen_coords, uv, depthBuffer, diffuseMap, lightIntensity, image);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] depthBuffer;
    return 0;
}