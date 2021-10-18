precision mediump float;
varying vec2 vTextureCoordinate;

uniform sampler2D sTextureSampler;
uniform sampler2D sBlurTextureSampler;
//强光程度, 默认值为24.0
const float intensity = 24.0;

void main() {
    lowp vec4 sourceColor = texture2D(sTextureSampler, vTextureCoordinate);
    lowp vec4 blurColor = texture2D(sBlurTextureSampler, vTextureCoordinate);
    highp vec4 highPassColor = sourceColor - blurColor;

    //对应混合模式中的强光模式(color = 2.0 * color1 * color2)，对于高反差的颜色来说，color1 和color2 是同一个
    highPassColor.r = clamp(2.0 * highPassColor.r * highPassColor.r * intensity, 0.0, 1.0);
    highPassColor.g = clamp(2.0 * highPassColor.g * highPassColor.g * intensity, 0.0, 1.0);
    highPassColor.b = clamp(2.0 * highPassColor.b * highPassColor.b * intensity, 0.0, 1.0);
    // 输出的是把痘印等过滤掉
    gl_FragColor = vec4(highPassColor.rgb, 1.0);
}