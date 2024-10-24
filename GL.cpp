#include "GL.h"
#include <algorithm>

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

GraphicsLibrary::GraphicsLibrary(int width, int height) : Output(width, height, TGAImage::RGB)
{
    ZBuffer = new float[width * height];
    for (int i = 0; i < width * height; ++i)
        ZBuffer[i] = std::numeric_limits<float>::lowest();
}

GraphicsLibrary::~GraphicsLibrary()
{
    delete[] ZBuffer;
}

void GraphicsLibrary::SetViewport(int x, int y, int w, int h, float depth)
{
    Viewport = Mat4::GetViewport(x, y, w, h, depth);
}

void GraphicsLibrary::SetProjection(float center)
{
    Projection = Mat4::GetProjection(center);
}

void GraphicsLibrary::LookAt(const Vec3f& position, const Vec3f& target, const Vec3f& up)
{
    ModelView = Mat4::LookAt(position, target, up);
}

Vec3f perspectiveProject(const Vec4f& vec)
{
    return { vec.x / vec.w, vec.y / vec.w, vec.z / vec.w };
}

void GraphicsLibrary::Triangle(Vertex vertices[3], Model& model, IShader& shader, Vec3f lightDirection)
{
    shader.GL = this;

    if (vertices[2].Pos.y == vertices[0].Pos.y)
        std::swap(vertices[0], vertices[1]);

    Vec2i min, max;
    Vec3f a = perspectiveProject(shader.VertexStage(vertices[0], 0));
    Vec3f b = perspectiveProject(shader.VertexStage(vertices[1], 1));
    Vec3f c = perspectiveProject(shader.VertexStage(vertices[2], 2));

    int width = Output.get_width();
    int height = Output.get_height();

    boundingbox(a, b, c, { width, height }, min, max);

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

                    if (ZBuffer[y * width + x] < depth)
                    {
                        TGAColor fragmentColor;
                        if (shader.FragmentStage({ sigma, alpha, beta }, fragmentColor))
                        {
                            ZBuffer[y * width + x] = depth;
                            Output.set(x, y, fragmentColor);
                        }
                    }
                }
            }
        }
    }
}