varying highp vec2 vTextureCoordinate;
uniform sampler2D sTextureSampler;

uniform lowp float uSaturation;
const mediump vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);
void main() {
    lowp vec4 textureColor = texture2D(sTextureSampler, vTextureCoordinate);
    lowp float luminance = dot(textureColor.rgb, luminanceWeighting);
    lowp vec3 greyScaleColor = vec3(luminance);
    gl_FragColor = vec4(mix(greyScaleColor, textureColor.rgb, uSaturation), textureColor.w);
}