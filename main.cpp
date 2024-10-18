#include "GL.h"
#include "matrix.h"
#include <iostream>
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

struct GouraudShader : public IShader
{
private:
    Vec3f varyingIntensity;
    Vec3f lightDirection;

public:
    GouraudShader(const GraphicsLibrary& glContext, const Vec3f& light) : IShader(glContext), lightDirection(light) {}

    virtual Vec4f VertexStage(const Vertex& vec, int vertexId) override
    {
        varyingIntensity.raw[vertexId] = std::max(0.f, vec.Normal * lightDirection);
        return GL.Viewport * GL.Projection * GL.ModelView * Vec4f(vec.Pos);
    }

    virtual bool FragmentStage(const Vec3f& bar, TGAColor& color) override
    {
        float intensity = bar * varyingIntensity;
        color = white * intensity;
        return true;
    }
};


int main(int argc, char** argv)
{
    const int windowWidth = 800;
    const int windowHeight = 800;

    GraphicsLibrary GL(windowWidth, windowHeight);

    Vec3f lightDirection = { 1.f, -1.f, 1.f };
    lightDirection.normalize();

    Vec3f cameraPos(1.f, 1.f, 3.f);
    Vec3f target(0.f, 0.f, 0.f);
    Vec3f up(0.f, 1.f, 0.f);

    float depth = (cameraPos - target).norm();

    GL.SetProjection(depth);
    GL.SetViewport(windowWidth / 8, windowHeight / 8, windowWidth * 3 / 4, windowHeight * 3 / 4, 255.f);
    GL.LookAt(cameraPos, target, up);
    Mat4 modelMatrix;

    Mat4 screenSpace = (GL.Viewport * GL.Projection * GL.ModelView * modelMatrix);

    Model model("african_head");
    if (!model.diffuseLoaded() || model.nverts() == 0)
    {
        std::cerr << "Error while loading model" << std::endl;
        return 1;
    }

    GouraudShader shader(GL, lightDirection);

    Vertex vertices[3];
    for (int i = 0; i < model.nfaces(); i++) 
    {
        std::vector<VertexInfo> face = model.face(i);

        for (int j = 0; j < 3; j++) 
        {
            vertices[j].Pos = model.vert(face[j].VertexId);
            vertices[j].UV = model.uv(face[j].TexCoordId);
            vertices[j].Normal = model.normal(face[j].NormalId);
        }

        GL.Triangle(vertices, model, shader, lightDirection);
    }

    GL.Output.flip_vertically();
    GL.Output.write_tga_file("output.tga");

    TGAImage zbuffer(windowWidth, windowHeight, TGAImage::RGB);

    float min = 100;
    float max = -100;

    for (int y = 0; y < windowHeight; ++y)
    {
        for (int x = 0; x < windowWidth; ++x)
        {
            float depth = GL.ZBuffer[windowWidth * y + x];
            if (depth < min)
                min = depth;

            if (depth > max)
                max = depth;

            zbuffer.set(x, y, TGAColor(depth, depth, depth, 255));
        }
    }

    zbuffer.flip_vertically();
    zbuffer.write_tga_file("depthbuffer.tga");

    return 0;
}