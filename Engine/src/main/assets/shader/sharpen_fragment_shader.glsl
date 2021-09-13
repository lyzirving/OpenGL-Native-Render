precision highp float;

varying highp vec2 varyingTextureCoordPos;
varying highp vec2 varyingLeftTextureCoordPos;
varying highp vec2 varyingRightTextureCoordPos;
varying highp vec2 varyingTopTextureCoordPos;
varying highp vec2 varyingBottomTextureCoordPos;

varying highp float varyingCenterMultiplier;
varying highp float varyingEdgeMultiplier;

uniform sampler2D sTextureSampler;

void main() {
    mediump vec3 textureColor = texture2D(sTextureSampler, varyingTextureCoordPos).rgb;
    mediump vec3 leftTextureColor = texture2D(sTextureSampler, varyingLeftTextureCoordPos).rgb;
    mediump vec3 rightTextureColor = texture2D(sTextureSampler, varyingRightTextureCoordPos).rgb;
    mediump vec3 topTextureColor = texture2D(sTextureSampler, varyingTopTextureCoordPos).rgb;
    mediump vec3 bottomTextureColor = texture2D(sTextureSampler, varyingBottomTextureCoordPos).rgb;

    gl_FragColor = vec4((textureColor * varyingCenterMultiplier - (leftTextureColor * varyingEdgeMultiplier + rightTextureColor * varyingEdgeMultiplier + topTextureColor * varyingEdgeMultiplier + bottomTextureColor * varyingEdgeMultiplier)), texture2D(sTextureSampler, varyingBottomTextureCoordPos).w);
}