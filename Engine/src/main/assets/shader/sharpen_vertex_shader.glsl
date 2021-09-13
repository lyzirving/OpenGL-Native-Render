attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;

uniform float uImgWidthFactor;
uniform float uImgHeightFactor;
uniform float uSharpness;

varying vec2 varyingTextureCoordPos;
varying vec2 varyingLeftTextureCoordPos;
varying vec2 varyingRightTextureCoordPos;
varying vec2 varyingTopTextureCoordPos;
varying vec2 varyingBottomTextureCoordPos;

varying float varyingCenterMultiplier;
varying float varyingEdgeMultiplier;

void main() {
    gl_Position = vec4(aVertexPos, 1);

    mediump vec2 widthStep = vec2(uImgWidthFactor, 0.0);
    mediump vec2 heightStep = vec2(0.0, uImgHeightFactor);

    varyingTextureCoordPos = aTextureCoordinate.xy;
    varyingLeftTextureCoordPos = aTextureCoordinate.xy - widthStep;
    varyingRightTextureCoordPos = aTextureCoordinate.xy + widthStep;
    varyingTopTextureCoordPos = aTextureCoordinate.xy + heightStep;
    varyingBottomTextureCoordPos = aTextureCoordinate.xy - heightStep;

    varyingCenterMultiplier = 1.0 + 4.0 * uSharpness;
    varyingEdgeMultiplier = uSharpness;
}
