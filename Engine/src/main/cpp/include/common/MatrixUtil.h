//
// Created by liuyuzhou on 2021/9/9.
//
#ifndef ENGINE_MATRIXUTIL_H
#define ENGINE_MATRIXUTIL_H

#include <GLES3/gl3.h>

class MatrixUtil {
public:
    static void flip(GLfloat* matrix, bool horizontalFlip, bool verticalFlip);
    static void move(GLfloat* src, GLfloat* dst, int size = 16);
    static void multiplyMM(GLfloat* resultM, const GLfloat* lhsM, const GLfloat* rhsM);
    static void orthogonal(GLfloat* matrix, int offset, GLfloat left, GLfloat right,
            GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
    static void rotate(GLfloat* matrix, GLfloat degree, GLfloat x, GLfloat y, GLfloat z);
    static void scaleM(GLfloat* matrix, int offset, float x, float y, float z);
    static void setRotate(GLfloat* rm, GLfloat degree, GLfloat x, GLfloat y, GLfloat z);
    static void setIdentityM(GLfloat* matrix, int offset);
private:
    static void logM(GLfloat* src);
};

#endif //ENGINE_MATRIXUTIL_H
