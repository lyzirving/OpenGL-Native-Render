//
// Created by liuyuzhou on 2021/9/9.
//
#ifndef ENGINE_MATRIXUTIL_H
#define ENGINE_MATRIXUTIL_H

#include <GLES2/gl2.h>

class MatrixUtil {
public:
    static void flip(GLfloat* matrix, bool horizontalFlip, bool verticalFlip);
    static void scaleM(GLfloat* matrix, int offset, float x, float y, float z);
    static void setIdentityM(GLfloat* matrix, int offset);
private:
};

#endif //ENGINE_MATRIXUTIL_H
