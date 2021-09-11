precision mediump float;
varying vec2 vTextureCoordinate;
uniform sampler2D sTextureSampler;
uniform float uContrast;
void main(){
    vec4 textureColor = texture2D(sTextureSampler, vTextureCoordinate);
    gl_FragColor = vec4(((textureColor.rgb - vec3(0.5)) * uContrast + vec3(0.5)), textureColor.w);
}