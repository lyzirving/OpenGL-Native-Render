//
// Created by liuyuzhou on 2021/10/21.
//
#ifndef ENGINE_OBJ3D_H
#define ENGINE_OBJ3D_H

#include <vector>
#include <string>

class Vertex {
public:
    Vertex();
    Vertex(double x, double y, double z);
    Vertex(const Vertex& vertex) noexcept;
    Vertex(Vertex&& vertex) noexcept;
    Vertex& operator=(const Vertex& vertex) noexcept;
    Vertex& operator=(Vertex&& vertex) noexcept;
    ~Vertex();

    double mX{0};
    double mY{0};
    double mZ{0};
};

class Texture {
public:
    Texture();
    Texture(double u, double v);
    Texture(const Texture& tex) noexcept;
    Texture(Texture&& tex) noexcept;
    Texture& operator=(const Texture& tex) noexcept;
    Texture& operator=(Texture&& tex) noexcept;
    ~Texture();

    double mU{0};
    double mV{0};
};

class VertexNormal {
public:
    VertexNormal();
    VertexNormal(double x, double y, double z);
    VertexNormal(const VertexNormal& normal) noexcept;
    VertexNormal(VertexNormal&& normal) noexcept;
    VertexNormal& operator=(const VertexNormal& normal) noexcept;
    VertexNormal& operator=(VertexNormal&& normal) noexcept;
    ~VertexNormal();

    double mX{0};
    double mY{0};
    double mZ{0};
};

class Obj3D {
public:
    Obj3D();
    ~Obj3D();

    void buildVertexArray(int count);
    void buildTextureArray(int count);
    void buildVertexNormalArray(int count);

    void setVertexArray(int index, double val);
    void setTextureArray(int index, double val);
    void setVertexNormalArray(int index, double val);
    void setMtlSrc(const std::string& name);

private:
    int mVertexCount{0};
    int mTextureCount{0};
    int mVertexNormalCount{0};
    float* mVertexArray{nullptr};
    float* mTextureArray{nullptr};
    float* mVertexNormalArray{nullptr};

    std::string mMtlSrcName{""};
};

#endif //ENGINE_OBJ3D_H
