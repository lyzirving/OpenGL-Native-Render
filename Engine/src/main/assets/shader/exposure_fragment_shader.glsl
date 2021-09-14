precision mediump float;
varying vec2 vTextureCoordinate;

uniform sampler2D sTextureSampler;
uniform highp float uExposure;

void main() {
    highp vec4 textureColor = texture2D(sTextureSampler, vTextureCoordinate);
    gl_FragColor = vec4(textureColor.rgb * pow(2.0, uExposure), textureColor.w);
}