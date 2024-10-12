#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "tgaimage.h"
#include "geometry.h"

struct VertexInfo
{
	int VertexId;
	int TexCoordId;
	int NormalId;
};

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<VertexInfo> > faces_;
	std::vector<Vec2f> uv_;
	std::vector<Vec3f> normals_;

	TGAImage diffuse_;
	bool diffuseLoaded_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nuv();
	Vec3f vert(int i);
	Vec2f uv(int i);
	Vec3f normal(int i);
	std::vector<VertexInfo> face(int idx);

	bool diffuseLoaded() const { return diffuseLoaded_; }
	TGAColor diffuse(Vec2f uv);
};

#endif //__MODEL_H__
