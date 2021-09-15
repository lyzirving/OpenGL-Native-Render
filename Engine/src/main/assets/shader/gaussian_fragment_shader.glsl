precision mediump float;
const int GAUSSIAN_SAMPLES = 9;

uniform sampler2D sTextureSampler;
varying vec2 vTextureCoordinate;
varying vec2 vBlurCoordinate[GAUSSIAN_SAMPLES];

void main() {
    vec3 sum = vec3(0.0);
    vec4 tmpFragmentColor = texture2D(sTextureSampler, vTextureCoordinate);

    sum += texture2D(sTextureSampler, vBlurCoordinate[0]).rgb * 0.05;
    sum += texture2D(sTextureSampler, vBlurCoordinate[1]).rgb * 0.09;
    sum += texture2D(sTextureSampler, vBlurCoordinate[2]).rgb * 0.12;
    sum += texture2D(sTextureSampler, vBlurCoordinate[3]).rgb * 0.15;
    sum += texture2D(sTextureSampler, vBlurCoordinate[4]).rgb * 0.18;
    sum += texture2D(sTextureSampler, vBlurCoordinate[5]).rgb * 0.15;
    sum += texture2D(sTextureSampler, vBlurCoordinate[6]).rgb * 0.12;
    sum += texture2D(sTextureSampler, vBlurCoordinate[7]).rgb * 0.09;
    sum += texture2D(sTextureSampler, vBlurCoordinate[8]).rgb * 0.05;

    gl_FragColor = vec4(sum, tmpFragmentColor.a);
}