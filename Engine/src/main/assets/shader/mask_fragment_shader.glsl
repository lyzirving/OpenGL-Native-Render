precision mediump float;
varying vec2 vTextureCoordinate;

uniform sampler2D sTextureSampler;
uniform sampler2D sMaskSampler;

void main() {
    highp vec4 sourceTexture = texture2D(sTextureSampler, vTextureCoordinate);
    highp vec4 maskTexture = texture2D(sMaskSampler, vTextureCoordinate);
    //black is source
    if(maskTexture.r == 0.0 && maskTexture.g == 0.0 && maskTexture.b == 0.0) {
        gl_FragColor = sourceTexture;
    } else {
        gl_FragColor = maskTexture;
    }
}