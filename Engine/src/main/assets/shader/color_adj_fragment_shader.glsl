precision mediump float;
const mediump vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);
uniform sampler2D sTextureSampler;
uniform float uContrast;
uniform float uSaturation;

varying vec2 vTextureCoordinate;
varying vec2 vLeftTextureCoordinate;
varying vec2 vRightTextureCoordinate;
varying vec2 vTopTextureCoordinate;
varying vec2 vBottomTextureCoordinate;

varying float vCenterMultiplier;
varying float vEdgeMultiplier;

vec4 caculateSharpen(vec3 currentColor, vec3 leftColor, vec3 rightColor, vec3 topColor, vec3 bottomColor, float alpha) {
    vec3 diffColor = leftColor * vEdgeMultiplier + rightColor * vEdgeMultiplier + topColor * vEdgeMultiplier + bottomColor * vEdgeMultiplier;
    return vec4((currentColor * vCenterMultiplier - diffColor), alpha);
}

vec4 caculateContrast(vec3 currentColor, float alpha) {
    return vec4(((currentColor - vec3(0.5)) * uContrast + vec3(0.5)), alpha);
}

vec4 caculateSaturation(vec3 currentColor, float alpha) {
    lowp float luminance = dot(currentColor, luminanceWeighting);
    lowp vec3 greyScaleColor = vec3(luminance);
    return vec4(mix(greyScaleColor, currentColor, uSaturation), alpha);
}

void main() {
    vec4 curTextureColor = texture2D(sTextureSampler, vTextureCoordinate);

    vec4 sharpenVal = caculateSharpen(curTextureColor.rgb,
    texture2D(sTextureSampler, vLeftTextureCoordinate).rgb,
    texture2D(sTextureSampler, vRightTextureCoordinate).rgb,
    texture2D(sTextureSampler, vTopTextureCoordinate).rgb,
    texture2D(sTextureSampler, vBottomTextureCoordinate).rgb,
    texture2D(sTextureSampler, vBottomTextureCoordinate).w);

    vec4 contrastVal = caculateContrast(sharpenVal.rgb, sharpenVal.w);

    gl_FragColor = caculateSaturation(contrastVal.rgb, contrastVal.w);
}