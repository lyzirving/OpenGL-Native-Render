//
// Created by liuyuzhou on 2021/10/21.
//
#include "Obj3D.h"

Vertex::Vertex() = default;

Vertex::Vertex(double x, double y, double z) : mX(x), mY(y), mZ(z) {}

Vertex::Vertex(const Vertex &vertex) noexcept : mX(vertex.mX), mY(vertex.mY), mZ(vertex.mZ) {}

Vertex::Vertex(Vertex &&vertex) noexcept : mX(vertex.mX), mY(vertex.mY), mZ(vertex.mZ)  {}

Vertex& Vertex::operator=(const Vertex &vertex) noexcept = default;

Vertex & Vertex::operator=(Vertex &&vertex) noexcept {
    if (this != &vertex) {
        this->mX = vertex.mX;
        this->mY = vertex.mY;
        this->mZ = vertex.mZ;
    }
    return *this;
}

Vertex::~Vertex() = default;

Texture::Texture() = default;

Texture::Texture(double u, double v) : mU(u), mV(v) {}

Texture::Texture(const Texture &tex) noexcept : mU(tex.mU), mV(tex.mV) {}

Texture::Texture(Texture &&tex) noexcept : mU(tex.mU), mV(tex.mV) {}

Texture& Texture::operator=(const Texture &tex) noexcept = default;

Texture & Texture::operator=(Texture &&tex) noexcept {
    if (this != &tex) {
        this->mU = tex.mU;
        this->mV = tex.mV;
    }
    return *this;
}

Texture::~Texture() = default;

VertexNormal::VertexNormal() = default;

VertexNormal::VertexNormal(double x, double y, double z) : mX(x), mY(y), mZ(z) {}

VertexNormal::VertexNormal(const VertexNormal &normal) noexcept : mX(normal.mX), mY(normal.mY), mZ(normal.mZ) {}

VertexNormal::VertexNormal(VertexNormal &&normal) noexcept : mX(normal.mX), mY(normal.mY), mZ(normal.mZ)  {}

VertexNormal& VertexNormal::operator=(const VertexNormal &normal) noexcept = default;

VertexNormal & VertexNormal::operator=(VertexNormal &&normal) noexcept {
    if (this != &normal) {
        this->mX = normal.mX;
        this->mY = normal.mY;
        this->mZ = normal.mZ;
    }
    return *this;
}

VertexNormal::~VertexNormal() = default;

Obj3D::Obj3D() {}

Obj3D::~Obj3D() {
    delete[] mVertexArray;
    delete[] mTextureArray;
    delete[] mVertexNormalArray;
}

void Obj3D::buildVertexArray(int count) {
    mVertexCount = count;
    delete[] mVertexArray;
    mVertexArray = new float[3 * count];
}

void Obj3D::buildTextureArray(int count) {
    mTextureCount = count;
    delete[] mTextureArray;
    mTextureArray = new float[3 * count];
}

void Obj3D::buildVertexNormalArray(int count) {
    mVertexNormalCount = count;
    delete[] mVertexNormalArray;
    mVertexNormalArray = new float[3 * count];
}

std::string Obj3D::getMtlSource() {
    return mMtlSrcName;
}

int Obj3D::getVertexCount() {
    return mVertexCount;
}

int Obj3D::getTextureCoordCount() {
    return mTextureCount;
}

int Obj3D::getVertexNormalCount() {
    return mVertexNormalCount;
}

MtlLib * Obj3D::getMtlLib() { return mMtlLib; }

float * Obj3D::getVertexArray() { return mVertexArray; }

float * Obj3D::getTextureArray() { return mTextureArray; }

float * Obj3D::getVertexNormalArray() { return mVertexNormalArray; }

void Obj3D::setVertexArray(int index, float val) {
    mVertexArray[index] = val;
}

void Obj3D::setTextureArray(int index, float val) {
    mTextureArray[index] = val;
}

void Obj3D::setVertexNormalArray(int index, float val) {
    mVertexNormalArray[index] = val;
}

void Obj3D::setMtlSrc(const std::string &name) { mMtlSrcName = name; }

void Obj3D::setMtlLib(MtlLib *lib) { mMtlLib = lib; }

