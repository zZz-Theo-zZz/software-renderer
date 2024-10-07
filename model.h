#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

struct VertexInfo
{
	int VertexId;
	int TexCoordId;
};

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<VertexInfo> > faces_;
	std::vector<Vec2f> uv_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nuv();
	Vec3f vert(int i);
	Vec2f uv(int i);
	std::vector<VertexInfo> face(int idx);
};

#endif //__MODEL_H__
