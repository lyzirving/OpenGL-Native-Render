precision mediump float;

uniform sampler2D sTextureSampler;
uniform sampler2D sBlurTextureSampler;//原图的高斯模糊纹理
uniform sampler2D sHighPassBlurTextureSampler;//高反差保留的高斯模糊纹理

uniform lowp float uSkinBuffIntensity;

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

vec4 caculateSkinBuff(vec4 srcColor, vec4 blurColor, vec4 highPassBlurColor) {
    //调节蓝色通道值
    mediump float value = clamp((min(srcColor.b, blurColor.b) - 0.2) * 5.0, 0.0, 1.0);
    //找到模糊之后RGB通道的最大值
    mediump float maxChannelColor = max(max(highPassBlurColor.r, highPassBlurColor.g), highPassBlurColor.b);
    //计算当前的强度
    mediump float currentIntensity = (1.0 - maxChannelColor / (maxChannelColor + 0.2)) * value * uSkinBuffIntensity;
    // 混合输出结果
    return vec4(mix(srcColor.rgb, blurColor.rgb, currentIntensity), srcColor.a);
}

void main() {
    vec4 mixedCurColor = caculateSkinBuff(texture2D(sTextureSampler, vTextureCoordinate),
    texture2D(sBlurTextureSampler, vTextureCoordinate),
    texture2D(sHighPassBlurTextureSampler, vTextureCoordinate));

    vec4 mixedLeftColor = caculateSkinBuff(texture2D(sTextureSampler, vLeftTextureCoordinate),
    texture2D(sBlurTextureSampler, vLeftTextureCoordinate),
    texture2D(sHighPassBlurTextureSampler, vLeftTextureCoordinate));

    vec4 mixedRightColor = caculateSkinBuff(texture2D(sTextureSampler, vRightTextureCoordinate),
    texture2D(sBlurTextureSampler, vRightTextureCoordinate),
    texture2D(sHighPassBlurTextureSampler, vRightTextureCoordinate));

    vec4 mixedTopColor = caculateSkinBuff(texture2D(sTextureSampler, vTopTextureCoordinate),
    texture2D(sBlurTextureSampler, vTopTextureCoordinate),
    texture2D(sHighPassBlurTextureSampler, vTopTextureCoordinate));

    vec4 mixedBottomColor = caculateSkinBuff(texture2D(sTextureSampler, vBottomTextureCoordinate),
    texture2D(sBlurTextureSampler, vBottomTextureCoordinate),
    texture2D(sHighPassBlurTextureSampler, vBottomTextureCoordinate));

    vec4 sharpenVal = caculateSharpen(mixedCurColor.rgb, mixedLeftColor.rgb, mixedRightColor.rgb,
    mixedTopColor.rgb, mixedBottomColor.rgb, mixedBottomColor.w);

    gl_FragColor = sharpenVal;
}