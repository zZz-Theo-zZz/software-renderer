#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<VertexInfo> f;
            VertexInfo vertex;
            int itrash;
            iss >> trash;
            while (iss >> vertex.VertexId >> trash >> vertex.TexCoordId >> trash >> itrash) {
                vertex.VertexId--; // in wavefront obj all indices start at 1, not zero
                vertex.TexCoordId--;
                f.push_back(vertex);
            }
            faces_.push_back(f);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            iss >> uv.x;
            iss >> uv.y;

            uv_.push_back(uv);
        }
    }
    std::cerr << "# v# " << verts_.size() << "# uv# " << uv_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::nuv()
{
    return (int)uv_.size();
}

std::vector<VertexInfo> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::uv(int i)
{
    return uv_[i];
}

