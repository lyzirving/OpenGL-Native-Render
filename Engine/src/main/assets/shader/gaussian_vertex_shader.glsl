const int GAUSSIAN_SAMPLES = 9;

attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;

uniform float uTextureWidthOffset;
uniform float uTextureHeightOffset;
uniform mat4 uMatrix;

varying vec2 vTextureCoordinate;
varying vec2 vBlurCoordinate[GAUSSIAN_SAMPLES];

void main() {
    gl_Position = uMatrix * vec4(aVertexPos,1);
    vTextureCoordinate = aTextureCoordinate;

    //Calculate the positions for the blur
    int multiplier = 0;
    vec2 blur_step;
    vec2 single_step_offset = vec2(uTextureWidthOffset, uTextureHeightOffset);
    for (int i = 0; i < GAUSSIAN_SAMPLES; i++) {
        multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));
        blur_step = float(multiplier) * single_step_offset;
        vBlurCoordinate[i] = aTextureCoordinate + blur_step;
    }
}
