precision mediump float;
const mediump vec3 luminanceWeighting = vec3(0.3, 0.3, 0.3);

varying vec2 vTextureCoordinate;
uniform sampler2D sTextureSampler;

uniform lowp float uShadows;
uniform lowp float uHighlights;

void main() {
    highp vec4 sourceTexture = texture2D(sTextureSampler, vTextureCoordinate);
    mediump float luminance = dot(sourceTexture.rgb, luminanceWeighting);
    mediump float shadow = clamp((pow(luminance, 1.0/(uShadows + 1.0)) + (-0.76) * pow(luminance, 2.0/(uShadows + 1.0))) - luminance, 0.0, 1.0);
    mediump float highlight = clamp((1.0 - (pow(1.0 - luminance, 1.0/(2.0 - uHighlights)) + (-0.8) * pow(1.0 - luminance, 2.0/(2.0 - uHighlights)))) - luminance, -1.0, 0.0);
    lowp vec3 result = vec3(0.0, 0.0, 0.0) + ((luminance + shadow + highlight) - 0.0) * ((sourceTexture.rgb - vec3(0.0, 0.0, 0.0))/(luminance - 0.0));
    gl_FragColor = vec4(result.rgb, sourceTexture.a);
}