precision mediump float;
uniform sampler2D sTextureSampler;

varying vec2 vTextureCoordinate;
varying vec4 vAmbient;
varying vec4 vDiffuse;

void main() {
    vec4 finalColor = texture2D(sTextureSampler, vTextureCoordinate);
    gl_FragColor = finalColor * (vDiffuse + vAmbient);
}