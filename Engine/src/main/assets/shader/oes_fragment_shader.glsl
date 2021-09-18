#extension GL_OES_EGL_image_external : require
precision mediump float;
varying vec2 vTextureCoordinate;
uniform samplerExternalOES sOesTextureSampler;
void main() {
    gl_FragColor = texture2D(sOesTextureSampler, vTextureCoordinate);
}