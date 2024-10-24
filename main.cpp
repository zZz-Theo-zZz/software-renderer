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
protected:
    Vec3f varyingIntensity;
    Vec3f lightDirection;

public:
    GouraudShader(const Vec3f& light) : lightDirection(light) {}

    virtual Vec4f VertexStage(const Vertex& vec, int vertexId) override
    {
        varyingIntensity.raw[vertexId] = std::max(0.f, vec.Normal * lightDirection);
        return GL->Viewport * GL->Projection * GL->ModelView * Vec4f(vec.Pos);
    }

    virtual bool FragmentStage(const Vec3f& bar, TGAColor& color) override
    {
        float intensity = bar * varyingIntensity;
        color = white * intensity;
        return true;
    }
};

struct TexturedGouraudShader : public IShader
{
protected:
    Vec3f varyingIntensity;
    Vec2f varyingUV[3];

    Vec3f lightDirection;
    Model& model;

public:
    TexturedGouraudShader(const Vec3f& light, Model& model) : lightDirection(light), model(model) {}

    virtual Vec4f VertexStage(const Vertex& vec, int vertexId) override
    {
        varyingUV[vertexId] =  vec.UV;
        
        varyingIntensity.raw[vertexId] = std::max(0.f, vec.Normal * lightDirection);
        return GL->Viewport * GL->Projection * GL->ModelView * Vec4f(vec.Pos);
    }

    virtual bool FragmentStage(const Vec3f& bar, TGAColor& color) override
    {
        float intensity = bar * varyingIntensity;
        Vec2f uv = varyingUV[0] * bar.x + varyingUV[1] * bar.y + varyingUV[2] * bar.z;
        TGAColor diffuse = model.diffuse(uv);
        color = diffuse * intensity;
        return true;
    }
};

struct BandShader : public GouraudShader
{
public:

    BandShader(const Vec3f& lightDirection) : GouraudShader(lightDirection) {}

    virtual bool FragmentStage(const Vec3f& bar, TGAColor& color) override {
        float intensity = varyingIntensity * bar;
        if (intensity > .85f) intensity = 1;
        else if (intensity > .60f) intensity = .80f;
        else if (intensity > .45f) intensity = .60f;
        else if (intensity > .30f) intensity = .45f;
        else if (intensity > .15f) intensity = .30f;
        else intensity = 0.f;
        color = white * intensity;
        return true;
    }
};

struct PhongShader : public IShader
{
protected:
    Vec2f varyingUV[3];
    Vec3f varyingNormal[3];

    Vec3f lightDirection;
    Model& model;

    Mat4 uniformModelView;
    Mat4 uniformModelViewInverseTranspose;

public:
    PhongShader(const Vec3f& light, Model& model, const Mat4& modelView, const Mat4& modelViewInverseTranspose) : 
        lightDirection(light), 
        model(model),
        uniformModelView(modelView),
        uniformModelViewInverseTranspose(modelViewInverseTranspose)
    {
        Vec4f vec = modelView * lightDirection;
        lightDirection = {vec.x, vec.y, vec.z};
    }

    virtual Vec4f VertexStage(const Vertex& vec, int vertexId) override
    {
        varyingUV[vertexId] = vec.UV;
        varyingNormal[vertexId] = vec.Normal;
        return GL->Viewport * GL->Projection * GL->ModelView * Vec4f(vec.Pos);
    }

    virtual bool FragmentStage(const Vec3f& bar, TGAColor& color) override
    {
        Vec2f uv = varyingUV[0] * bar.x + varyingUV[1] * bar.y + varyingUV[2] * bar.z;
        Vec4f tmp = uniformModelViewInverseTranspose * model.normal(uv);
        Vec3f normal = Vec3f{ tmp.x, tmp.y, tmp.z };
        normal = normal.normalize();

        float intensity = normal * lightDirection;
        if (intensity > 0.0f)
        {
            color = model.diffuse(uv) * intensity;
            return true;
        }

        return false;
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

    Model model("african_head");
    if (!model.diffuseLoaded() || !model.normalLoaded() || model.nverts() == 0)
    {
        std::cerr << "Error while loading model" << std::endl;
        return 1;
    }

    Mat4 inverseTranspose = Mat4::Transpose((GL.Projection * GL.ModelView).Inverse());

    GouraudShader shader(lightDirection);
    TexturedGouraudShader texturedGouraud(lightDirection, model);
    BandShader bandShader(lightDirection);
    PhongShader phongShader(lightDirection, model, GL.Projection * GL.ModelView, inverseTranspose);

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

        GL.Triangle(vertices, model, phongShader, lightDirection);
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
            unsigned char depth = (unsigned char)GL.ZBuffer[windowWidth * y + x];
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