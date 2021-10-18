precision mediump float;
varying vec2 vTextureCoordinate;

uniform sampler2D sTextureSampler;
uniform sampler2D sGrayTextureSampler;//灰度查找表
uniform sampler2D sLookupTextureSampler;// LUT

uniform highp float uLevelRangeInv;//范围
uniform lowp float uLevelBlack; //灰度level
uniform lowp float uAlpha; //肤色程度

void main() {
    lowp vec3 textureColor = texture2D(sTextureSampler, vTextureCoordinate).rgb;
    //genType clamp(genType x, genType minVal, genType maxVal)
    //clamp is used to find the middle value from the three input
    textureColor = clamp((textureColor - vec3(uLevelBlack, uLevelBlack, uLevelBlack)) * uLevelRangeInv, 0.0, 1.0);
    textureColor.r = texture2D(sGrayTextureSampler, vec2(textureColor.r, 0.5)).r;
    textureColor.g = texture2D(sGrayTextureSampler, vec2(textureColor.g, 0.5)).g;
    textureColor.b = texture2D(sGrayTextureSampler, vec2(textureColor.b, 0.5)).b;

    mediump float blueColor = textureColor.b * 15.0;

    mediump vec2 quad1;
    quad1.y = floor(blueColor / 4.0);
    quad1.x = floor(blueColor) - (quad1.y * 4.0);

    mediump vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 4.0);
    quad2.x = ceil(blueColor) - (quad2.y * 4.0);

    highp vec2 texPos1;
    texPos1.x = (quad1.x * 0.25) + 0.5 / 64.0 + ((0.25 - 1.0 / 64.0) * textureColor.r);
    texPos1.y = (quad1.y * 0.25) + 0.5 / 64.0 + ((0.25 - 1.0 / 64.0) * textureColor.g);

    highp vec2 texPos2;
    texPos2.x = (quad2.x * 0.25) + 0.5 / 64.0 + ((0.25 - 1.0 / 64.0) * textureColor.r);
    texPos2.y = (quad2.y * 0.25) + 0.5 / 64.0 + ((0.25 - 1.0 / 64.0) * textureColor.g);

    lowp vec4 newColor1 = texture2D(sLookupTextureSampler, texPos1);
    lowp vec4 newColor2 = texture2D(sLookupTextureSampler, texPos2);

    lowp vec3 newColor = mix(newColor1.rgb, newColor2.rgb, fract(blueColor));

    //mix(x,y,a), return the result of x(1-a) + y*a
    textureColor = mix(textureColor, newColor, uAlpha);

    gl_FragColor = vec4(textureColor, 1.0);
}