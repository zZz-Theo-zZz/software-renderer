#pragma once

#include "model.h"
#include "tgaimage.h"
#include "geometry.h"
#include "matrix.h"

struct Vertex
{
    Vec3f Pos;
    Vec3f Normal;
    Vec2f UV;
};

struct IShader;

class GraphicsLibrary
{
public:

    float* ZBuffer;
    TGAImage Output;

    Mat4 ModelView;
    Mat4 Viewport;
    Mat4 Projection;

    GraphicsLibrary(int width, int height);
    ~GraphicsLibrary();

    void SetViewport(int x, int y, int w, int h, float depth);

    void SetProjection(float center);

    void LookAt(const Vec3f& position, const Vec3f& target, const Vec3f& up);

	void Triangle(Vertex vertices[3], Model& model, IShader& shader, Vec3f lightDirection);
};

struct IShader
{
    const GraphicsLibrary& GL;
    IShader(const GraphicsLibrary& glContext) : GL(glContext) {}
    virtual ~IShader() {}
    virtual Vec4f VertexStage(const Vertex& vec, int vertexId) = 0;
    virtual bool FragmentStage(const Vec3f& bar, TGAColor& color) = 0;
};