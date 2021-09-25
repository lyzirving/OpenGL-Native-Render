//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_BASEFILTER_H
#define ENGINE_BASEFILTER_H

#include <GLES2/gl2.h>

class BaseFilter {
public:
    static const GLint DEFAULT_VERTEX_COUNT = 6;

    BaseFilter();
    ~BaseFilter();

    virtual void adjust(int progress);
    virtual void destroy();
    virtual const char* getType();
    virtual void init();
    virtual void initHandler();
    virtual void initBuffer();
    virtual void initTexture();
    virtual void initFrameBuffer();

    virtual void loadShader();
    virtual GLint onDraw(GLint inputTextureId);
    virtual GLint onDraw(GLint inputFrameBufferId, GLint inputTextureId);
    virtual void onPause();
    virtual void onResume();
    virtual void preInit();
    virtual void postInit();
    virtual void setOutputSize(GLint width, GLint height);

    virtual bool initialized();

protected:
    char* mFilterType = nullptr;
    bool mInitialized{false};
    GLuint mProgram{0};
    const char* mVertexShader = nullptr;
    const char* mTextureShader = nullptr;

    GLint mVertexPosHandler{0};
    GLint mTextureCoordinateHandler{0};
    GLint mTextureSamplerHandler{0};

    /** how the vertexes are arranged on screen
     *  0 means center of screen
     *   3-----------1(4)
     *   |         | |
     *   |       |   |
     *   |   （0）    |
     *   |   |       |
     *   | |         |
     *   2(6)--------5
     */
    GLfloat *mVertex = nullptr;

    /** how the texture coordinate are arranged on screen
     * (0,0)------------------(1,0)
     *   |                   |  |
     *   |   center of screen   |
     *   |       (0.5,0.5)      |
     *   |       |              |
     *   |   |                  |
     * (0,1)------------------(1,1)
     */
    GLfloat *mTextureCoordinate = nullptr;

    GLfloat *mMatrix = nullptr;

    GLint mWidth{0};
    GLint mHeight{0};

    GLuint mBufferId[2]{0, 0};
    GLuint mTextureId{0};
private:
};

#endif //ENGINE_BASEFILTER_H
