#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), diffuseLoaded_(false) {
    std::ifstream in;

    std::string path = filename;
    path.append(".obj");

    in.open (path.c_str(), std::ifstream::in);
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
            iss >> trash;
            while (iss >> vertex.VertexId >> trash >> vertex.TexCoordId >> trash >> vertex.NormalId) {
                vertex.VertexId--; // in wavefront obj all indices start at 1, not zero
                vertex.TexCoordId--;
                vertex.NormalId--;
                f.push_back(vertex);
            }
            faces_.push_back(f);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            iss >> uv.x;
            iss >> uv.y;

            uv_.push_back(uv);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash;
            iss >> trash;

            Vec3f normal;
            iss >> normal.x;
            iss >> normal.y;
            iss >> normal.z;

            normals_.push_back(normal);
        }
    }
    std::cerr << "# v# " << verts_.size() << "# uv# " << uv_.size() << " f# " << faces_.size() << std::endl;


    std::string diffusePath = filename;
    diffusePath.append("_diffuse.tga");

    diffuseLoaded_ = true;
    if (!diffuse_.read_tga_file(diffusePath.c_str()))
    {
        std::cerr << "Couldn't read diffuse map " << diffusePath << std::endl;
        diffuseLoaded_ = false;
    }

    std::string normalPath = filename;
    normalPath.append("_normal.tga");
    normalLoaded_ = true;
    if (!normal_.read_tga_file(normalPath.c_str()))
    {
        std::cerr << "Couldn't read normal map " << normalPath << std::endl;
        normalLoaded_ = false;
    }

    std::string specularPath = filename;
    specularPath.append("_spec.tga");
    specularLoaded_ = true;
    if (!specular_.read_tga_file(specularPath.c_str()))
    {
        std::cerr << "Couldn't read specular map " << specularPath << std::endl;
        specularLoaded_ = false;
    }
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

Vec3f Model::normal(int i)
{
    return normals_[i];
}

TGAColor Model::diffuse(Vec2f uv)
{
    return diffuse_.get((int)(uv.x * diffuse_.get_width()), (int)((1.0f - uv.y) * diffuse_.get_height()));
}

Vec3f Model::normal(Vec2f uv)
{
    TGAColor color = normal_.get((int)(uv.x * normal_.get_width()), (int)((1.0f - uv.y) * normal_.get_height()));

    Vec3f vec = { (float)color.r, (float)color.g, (float)color.b };
    vec.normalize();

    return vec;
}

float Model::specular(Vec2f uv)
{
    TGAColor color = specular_.get((int)(uv.x * specular_.get_width()), (int)((1.0f - uv.y) * specular_.get_height()));

    return color.b;
}

