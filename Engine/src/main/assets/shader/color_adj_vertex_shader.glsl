attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;

uniform float uImgWidthFactor;
uniform float uImgHeightFactor;
uniform float uSharpness;

varying vec2 vTextureCoordinate;
varying vec2 vLeftTextureCoordinate;
varying vec2 vRightTextureCoordinate;
varying vec2 vTopTextureCoordinate;
varying vec2 vBottomTextureCoordinate;

varying float vCenterMultiplier;
varying float vEdgeMultiplier;

void main() {
    gl_Position = vec4(aVertexPos,1.0);

    mediump vec2 widthStep = vec2(uImgWidthFactor, 0.0);
    mediump vec2 heightStep = vec2(0.0, uImgHeightFactor);

    vTextureCoordinate = aTextureCoordinate;
    //for sharpen
    vLeftTextureCoordinate = aTextureCoordinate.xy - widthStep;
    vRightTextureCoordinate = aTextureCoordinate.xy + widthStep;
    vTopTextureCoordinate = aTextureCoordinate.xy + heightStep;
    vBottomTextureCoordinate = aTextureCoordinate.xy - heightStep;
    vCenterMultiplier = 1.0 + 4.0 * uSharpness;
    vEdgeMultiplier = uSharpness;
    //for sharpen end
}
