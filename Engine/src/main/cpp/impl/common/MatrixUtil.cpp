//
// Created by liuyuzhou on 2021/9/9.
//
#include <cmath>
#include "MatrixUtil.h"
#include "LogUtil.h"

#define I(_i, _j) ((_j)+ 4*(_i))

GLfloat* gTempM = nullptr;

class MatrixException : public std::exception {
public:
    explicit MatrixException(const char* input) { msg = input; }
    const char *what() const _NOEXCEPT override { return msg; }
private:
    const char* msg{nullptr};
};

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

GLfloat MatrixUtil::length(GLfloat x, GLfloat y, GLfloat z) {
    return std::sqrt(x * x + y * y +  z * z);
}

void MatrixUtil::move(GLfloat *src, GLfloat *dst, int size) {
    for (int i = 0; i < size; ++i) { dst[i] = src[i]; }
}

void MatrixUtil::multiplyMM(GLfloat *resultM, const GLfloat *lhsM, const GLfloat *rhsM) {
    for (int i = 0; i < 4; i++) {
        const float rhs_i0 = rhsM[I(i, 0)];
        float ri0 = lhsM[I(0, 0)] * rhs_i0;
        float ri1 = lhsM[I(0, 1)] * rhs_i0;
        float ri2 = lhsM[I(0, 2)] * rhs_i0;
        float ri3 = lhsM[I(0, 3)] * rhs_i0;
        for (int j = 1; j < 4; j++) {
            const float rhs_ij = rhsM[I(i, j)];
            ri0 += lhsM[I(j, 0)] * rhs_ij;
            ri1 += lhsM[I(j, 1)] * rhs_ij;
            ri2 += lhsM[I(j, 2)] * rhs_ij;
            ri3 += lhsM[I(j, 3)] * rhs_ij;
        }
        resultM[I(i, 0)] = ri0;
        resultM[I(i, 1)] = ri1;
        resultM[I(i, 2)] = ri2;
        resultM[I(i, 3)] = ri3;
    }
}

void MatrixUtil::orthogonal(GLfloat *matrix, int offset, GLfloat left, GLfloat right,
        GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    if (left == right) {
        throw MatrixException("orthogonal: left should not equal to right");
    }
    if (bottom == top) {
        throw MatrixException("orthogonal: bottom should not equal to top");
    }
    if (near == far) {
        throw MatrixException("orthogonal: near should not equal to far");
    }
    GLfloat r_width  = 1.0f / (right - left);
    GLfloat r_height = 1.0f / (top - bottom);
    GLfloat r_depth  = 1.0f / (far - near);
    GLfloat x =  2.0f * (r_width);
    GLfloat y =  2.0f * (r_height);
    GLfloat z = -2.0f * (r_depth);
    GLfloat tx = -(right + left) * r_width;
    GLfloat ty = -(top + bottom) * r_height;
    GLfloat tz = -(far + near) * r_depth;
    matrix[offset + 0] = x;
    matrix[offset + 5] = y;
    matrix[offset +10] = z;
    matrix[offset +12] = tx;
    matrix[offset +13] = ty;
    matrix[offset +14] = tz;
    matrix[offset +15] = 1.0f;
    matrix[offset + 1] = 0.0f;
    matrix[offset + 2] = 0.0f;
    matrix[offset + 3] = 0.0f;
    matrix[offset + 4] = 0.0f;
    matrix[offset + 6] = 0.0f;
    matrix[offset + 7] = 0.0f;
    matrix[offset + 8] = 0.0f;
    matrix[offset + 9] = 0.0f;
    matrix[offset + 11] = 0.0f;
}

void MatrixUtil::rotate(GLfloat *matrix, GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
    if (gTempM == nullptr) {
        gTempM = new GLfloat[16];
        MatrixUtil::setIdentityM(gTempM, 0);
    }
    setRotate(gTempM, degree, x, y, z);
    auto* result = new GLfloat[16];
    multiplyMM(result, matrix, gTempM);
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

void MatrixUtil::setLookAt(GLfloat *rm, int rmOffset, GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
                           GLfloat centerX, GLfloat centerY, GLfloat centerZ,
                           GLfloat upX, GLfloat upY, GLfloat upZ) {
    // See the OpenGL GLUT documentation for gluLookAt for a description
    // of the algorithm. We implement it in a straightforward way:

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / length(fx, fy, fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // and normalize s
    float rls = 1.0f / length(sx, sy, sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    rm[rmOffset + 0] = sx;
    rm[rmOffset + 1] = ux;
    rm[rmOffset + 2] = -fx;
    rm[rmOffset + 3] = 0.0f;

    rm[rmOffset + 4] = sy;
    rm[rmOffset + 5] = uy;
    rm[rmOffset + 6] = -fy;
    rm[rmOffset + 7] = 0.0f;

    rm[rmOffset + 8] = sz;
    rm[rmOffset + 9] = uz;
    rm[rmOffset + 10] = -fz;
    rm[rmOffset + 11] = 0.0f;

    rm[rmOffset + 12] = 0.0f;
    rm[rmOffset + 13] = 0.0f;
    rm[rmOffset + 14] = 0.0f;
    rm[rmOffset + 15] = 1.0f;

    translateM(rm, rmOffset, -eyeX, -eyeY, -eyeZ);
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

void MatrixUtil::translateM(GLfloat *matrix, int offset, float x, float y, float z) {
    for (int i = 0; i < 4; ++i) {
        int mi = offset + i;
        matrix[12 + mi] += matrix[mi] * x + matrix[4 + mi] * y + matrix[8 + mi] * z;
    }
}

