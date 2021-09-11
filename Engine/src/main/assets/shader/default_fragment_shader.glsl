precision mediump float;
varying vec2 vTextureCoordinate;
uniform sampler2D sTextureSampler;

void main() {
    gl_FragColor = texture2D(sTextureSampler, vTextureCoordinate);
}