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

                        Vec2f currentPixelUV = vertices[0].UV * sigma + vertices[1].UV * alpha + vertices[2].UV * beta;
                        Vec3f pixelNormal = vertices[0].Normal * sigma + vertices[1].Normal * alpha + vertices[2].Normal * beta;
                        pixelNormal.normalize();

                        float light = pixelNormal * lightDirection;
                        TGAColor diffuse = model.diffuse(currentPixelUV);
                        //TGAColor normalSampleColor(pixelNormal.x * 255, pixelNormal.y * 255, pixelNormal.z * 255, 255);

                        //float depthColor = (depth + 1) * 0.5f;
                        //TGAColor depthSampleColor(depthColor * 255, depthColor * 255, depthColor * 255, 255);

                        output.set(x, y, diffuse * light);
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

    float c = 5.0f;
    float projectionData[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, -1.0f / c, 1.0f
    };

    float modelMatrixData[16] =
    {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, -1.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };

    Mat4 projectionMatrix(projectionData);
    Mat4 modelMatrix(modelMatrixData);

    float* depthBuffer = new float[windowWidth * windowHeight];
    for (int i = 0; i < windowHeight * windowWidth; ++i)
        depthBuffer[i] = -std::numeric_limits<float>::max();

    TGAImage output(windowWidth, windowHeight, TGAImage::RGB);

    Model model("african_head");
    if (!model.diffuseLoaded() || model.nverts() == 0)
    {
        std::cerr << "Error while loading model" << std::endl;
        return 1;
    }

    Vec3f lightDirection = { 0.f, 0.f, 1.0f};

    Vertex vertices[3];
    for (int i = 0; i < model.nfaces(); i++) 
    {
        std::vector<VertexInfo> face = model.face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];

        for (int j = 0; j < 3; j++) 
        {
            Vec3f vertex = model.vert(face[j].VertexId);
            Vec4f projectedCoords = projectionMatrix * modelMatrix * Vec4f(vertex);

            screen_coords[j] = Vec3f(projectedCoords.x / projectedCoords.w, projectedCoords.y / projectedCoords.w, vertex.z);
            screen_coords[j] = Vec3f((screen_coords[j].x + 1.f) * windowWidth / 2.f, (screen_coords[j].y + 1.f) * windowHeight / 2.f, vertex.z);
            
            world_coords[j] = vertex;

            vertices[j].Pos = screen_coords[j];
            vertices[j].UV = model.uv(face[j].TexCoordId);
            vertices[j].Normal = model.normal(face[j].NormalId);
        }

        Vec3f normal = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        normal.normalize();

        triangle(vertices, model, depthBuffer, lightDirection, output);
    }

    output.flip_vertically();
    output.write_tga_file("output.tga");

    delete[] depthBuffer;
    return 0;
}