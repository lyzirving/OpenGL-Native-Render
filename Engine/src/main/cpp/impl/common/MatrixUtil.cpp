//
// Created by liuyuzhou on 2021/9/9.
//
#include <cmath>
#include "MatrixUtil.h"
#include "LogUtil.h"

#define I(_i, _j) ((_j)+ 4*(_i))

GLfloat* gTempM = nullptr;

void MatrixUtil::flip(GLfloat *matrix, bool horizontalFlip, bool verticalFlip) {
    if (horizontalFlip || verticalFlip) {
        scaleM(matrix, 0, horizontalFlip ? -1 : 1, verticalFlip ? -1 : 1, 1);
    }
}

void MatrixUtil::logM(GLfloat *src) {
    LogUtil::logI("Matrix", {"input matrix:\n",
                             std::to_string(src[0]), " ", std::to_string(src[1]), " ", std::to_string(src[2]), " ", std::to_string(src[3]), "\n",
                             std::to_string(src[4]), " ", std::to_string(src[5]), " ", std::to_string(src[6]), " ", std::to_string(src[7]), "\n",
                             std::to_string(src[9]), " ", std::to_string(src[9]), " ", std::to_string(src[10]), " ", std::to_string(src[11]), "\n",
                             std::to_string(src[12]), " ", std::to_string(src[13]), " ", std::to_string(src[14]), " ", std::to_string(src[15]), "\n"});
}

void MatrixUtil::move(GLfloat *src, GLfloat *dst, int size) {
    for (int i = 0; i < size; ++i) { dst[i] = src[i]; }
}

void MatrixUtil::multiply(GLfloat *rm, GLfloat *lhs, GLfloat *rhs) {
    for (int i = 0; i < 4; i++) {
        const float rhs_i0 = rhs[I(i, 0)];
        float ri0 = lhs[I(0, 0)] * rhs_i0;
        float ri1 = lhs[I(0, 1)] * rhs_i0;
        float ri2 = lhs[I(0, 2)] * rhs_i0;
        float ri3 = lhs[I(0, 3)] * rhs_i0;
        for (int j = 1; j < 4; j++) {
            const float rhs_ij = rhs[I(i, j)];
            ri0 += lhs[I(j, 0)] * rhs_ij;
            ri1 += lhs[I(j, 1)] * rhs_ij;
            ri2 += lhs[I(j, 2)] * rhs_ij;
            ri3 += lhs[I(j, 3)] * rhs_ij;
        }
        rm[I(i, 0)] = ri0;
        rm[I(i, 1)] = ri1;
        rm[I(i, 2)] = ri2;
        rm[I(i, 3)] = ri3;
    }
}

void MatrixUtil::rotate(GLfloat *matrix, GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
    if (gTempM == nullptr) {
        gTempM = new GLfloat[16];
        MatrixUtil::setIdentityM(gTempM, 0);
    }
    setRotate(gTempM, degree, x, y, z);
    auto* result = new GLfloat[16];
    multiply(result, matrix, gTempM);
    move(result, matrix);
    delete[] result;
}

void MatrixUtil::scaleM(GLfloat *matrix, int offset, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
        int mi = offset + i;
        matrix[mi] *= x;
        matrix[4 + mi] *= y;
        matrix[8 + mi] *= z;
    }
}

void MatrixUtil::setIdentityM(GLfloat *matrix, int offset) {
    for (int i = 0; i < 16; i++) {
        matrix[offset + i] = 0;
    }
    for (int i = 0; i < 16; i += 5) {
        matrix[offset + i] = 1.0f;
    }
}

void MatrixUtil::setRotate(GLfloat *rm, GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
    rm[3] = 0;
    rm[7] = 0;
    rm[11]= 0;
    rm[12]= 0;
    rm[13]= 0;
    rm[14]= 0;
    rm[15]= 1;
    degree *= (GLfloat) (M_PI / 180);
    auto s = (GLfloat) sin(degree);
    auto c = (GLfloat) cos(degree);
    if (1 == x && 0 == y && 0 == z) {
        rm[5] = c;   rm[10]= c;
        rm[6] = s;   rm[9] = -s;
        rm[1] = 0;   rm[2] = 0;
        rm[4] = 0;   rm[8] = 0;
        rm[0] = 1;
    } else if (0 == x && 1 == y && 0 == z) {
        rm[0] = c;   rm[10]= c;
        rm[8] = s;   rm[2] = -s;
        rm[1] = 0;   rm[4] = 0;
        rm[6] = 0;   rm[9] = 0;
        rm[5] = 1;
    } else if (0. == x && 0 == y && 1 == z) {
        rm[0] = c;   rm[5] = c;
        rm[1] = s;   rm[4] = -s;
        rm[2] = 0;   rm[6] = 0;
        rm[8] = 0;   rm[9] = 0;
        rm[10]= 1;
    } else {
        GLfloat len = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
        if (1.0 != len) {
            GLfloat reverseLen = 1.0 / len;
            x *= reverseLen;
            y *= reverseLen;
            z *= reverseLen;
        }
        GLfloat nc = 1.0 - c;
        GLfloat xy = x * y;
        GLfloat yz = y * z;
        GLfloat zx = z * x;
        GLfloat xs = x * s;
        GLfloat ys = y * s;
        GLfloat zs = z * s;
        rm[0] = x*x*nc +  c;
        rm[4] =  xy*nc - zs;
        rm[8] =  zx*nc + ys;
        rm[1] =  xy*nc + zs;
        rm[5] = y*y*nc +  c;
        rm[9] =  yz*nc - xs;
        rm[2] =  zx*nc - ys;
        rm[6] =  yz*nc + xs;
        rm[10] = z*z*nc +  c;
    }
}

