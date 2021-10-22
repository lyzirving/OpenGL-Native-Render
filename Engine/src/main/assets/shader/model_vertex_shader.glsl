attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;
attribute vec3 aVertexNormal;

uniform mat4 uMVPMatrix;
uniform mat4 uMMatrix;
uniform vec3 uLightLocation;
uniform vec3 aAmbient;
uniform vec3 uDiffuse;

varying vec2 vTextureCoordinate;
varying vec4 vAmbient;
varying vec4 vDiffuse;

void main() {
    gl_Position = uMVPMatrix * vec4(aVertexPos, 1.0);
    vTextureCoordinate = aTextureCoordinate;

    vec3 transformedNormal = normalize((uMMatrix * vec4(aVertexNormal, 0.0)).xyz);
    vec3 lightVector= normalize(uLightLocation - (uMMatrix * vec4(aVertexPos, 1.0)).xyz);
    float diffuseFactor = max(0.1, -dot(transformedNormal, lightVector));

    vDiffuse = vec4((uDiffuse * diffuseFactor).xyz, 1.0);
    vAmbient = vec4(aAmbient, 1);
}
