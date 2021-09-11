//
// Created by liuyuzhou on 2021/9/9.
//
#include "MatrixUtil.h"

void MatrixUtil::flip(GLfloat *matrix, bool horizontalFlip, bool verticalFlip) {
    if (horizontalFlip || verticalFlip) {
        scaleM(matrix, 0, horizontalFlip ? -1 : 1, verticalFlip ? -1 : 1, 1);
    }
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

