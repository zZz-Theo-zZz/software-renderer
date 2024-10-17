#include "tgaimage.h"
#include "model.h"
#include "matrix.h"
#include <iostream>
#include <algorithm>
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);


struct Vertex
{
    Vec3f Pos;
    Vec3f Normal;
    Vec2f UV;
};

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

    min = { std::min((int)t0.x, min.x), std::min((int)t0.y, min.y) };
    min = { std::min((int)t1.x, min.x), std::min((int)t1.y, min.y) };
    min = { std::min((int)t2.x, min.x), std::min((int)t2.y, min.y) };

    max = { std::max((int)t0.x, max.x), std::max((int)t0.y, max.y) };
    max = { std::max((int)t1.x, max.x), std::max((int)t1.y, max.y) };
    max = { std::max((int)t2.x, max.x), std::max((int)t2.y, max.y) };

    min = { std::clamp(min.x, 0, windowSize.x - 1), std::clamp(min.y, 0, windowSize.y - 1) };
    max = { std::clamp(max.x, 0, windowSize.x - 1), std::clamp(max.y, 0, windowSize.y - 1) };
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

void triangle(Vertex vertices[3], Model& model, float* depthBuffer, Vec3f lightDirection, TGAImage& output)
{
    Vec2i min, max;
    Vec3f a = vertices[0].Pos;
    Vec3f b = vertices[1].Pos;
    Vec3f c = vertices[2].Pos;

    int width = output.get_width();
    int height = output.get_height();

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

                        Vec3f pixelNormal = vertices[0].Normal * sigma + vertices[1].Normal * alpha + vertices[2].Normal * beta;
                        pixelNormal.normalize();
                        float light = pixelNormal * lightDirection;

                        if (light > 0.0f)
                        {
                            Vec2f currentPixelUV = vertices[0].UV * sigma + vertices[1].UV * alpha + vertices[2].UV * beta;
                            TGAColor diffuse = model.diffuse(currentPixelUV);

                            output.set(x, y, diffuse * light);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    const int windowWidth = 800;
    const int windowHeight = 800;

    Vec3f lightDirection = { 1.f, -1.f, 1.f };
    lightDirection.normalize();

    Vec3f cameraPos(1.f, 1.f, 3.f);
    Vec3f target(0.f, 0.f, 0.f);
    Vec3f up(0.f, 1.f, 0.f);

    float depth = (cameraPos - target).norm();

    Mat4 projection = Mat4::GetProjection(depth);
    Mat4 viewport = Mat4::GetViewport(windowWidth / 8, windowHeight / 8, windowWidth * 3 / 4, windowHeight * 3 / 4, 255.f);
    Mat4 view = Mat4::LookAt(cameraPos, target, up);
    Mat4 modelMatrix;

    std::cerr << view << std::endl;
    std::cerr << projection << std::endl;
    std::cerr << viewport << std::endl;
    Mat4 z = (viewport * projection * view);
    std::cerr << z << std::endl;

    float* depthBuffer = new float[windowWidth * windowHeight];
    for (int i = 0; i < windowHeight * windowWidth; ++i)
        depthBuffer[i] = std::numeric_limits<float>::lowest();

    TGAImage output(windowWidth, windowHeight, TGAImage::RGB);

    Model model("african_head");
    if (!model.diffuseLoaded() || model.nverts() == 0)
    {
        std::cerr << "Error while loading model" << std::endl;
        return 1;
    }

    Vertex vertices[3];
    for (int i = 0; i < model.nfaces(); i++) 
    {
        std::vector<VertexInfo> face = model.face(i);

        for (int j = 0; j < 3; j++) 
        {
            Vec3f vertex = model.vert(face[j].VertexId);
            Vec4f projectedCoords = viewport * projection * view * modelMatrix * Vec4f(vertex);

            vertices[j].Pos = Vec3f(projectedCoords.x / projectedCoords.w, projectedCoords.y / projectedCoords.w, projectedCoords.z / projectedCoords.w);
            vertices[j].UV = model.uv(face[j].TexCoordId);
            vertices[j].Normal = model.normal(face[j].NormalId);
        }

        triangle(vertices, model, depthBuffer, lightDirection, output);
    }

    output.flip_vertically();
    output.write_tga_file("output.tga");


    TGAImage zbuffer(windowWidth, windowHeight, TGAImage::RGB);

    float min = 100;
    float max = -100;

    for (int y = 0; y < windowHeight; ++y)
    {
        for (int x = 0; x < windowWidth; ++x)
        {
            float depth = depthBuffer[windowWidth * y + x];
            if (depth < min)
                min = depth;

            if (depth > max)
                max = depth;

            zbuffer.set(x, y, TGAColor(depth, depth, depth, 255));
        }
    }

    zbuffer.flip_vertically();
    zbuffer.write_tga_file("depthbuffer.tga");

    delete[] depthBuffer;
    return 0;
}