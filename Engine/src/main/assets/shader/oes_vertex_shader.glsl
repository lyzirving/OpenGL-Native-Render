attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;
uniform vec4 uOesTexMatrix;
varying vec2 vTextureCoordinate;

void main() {
    gl_Position = vec4(aVertexPos, 1.0);
    vec4 texVec4 = vec4(aTextureCoordinate.x, aTextureCoordinate.y, 0.0, 1.0);
//    vTextureCoordinate = (uOesTexMatrix * texVec4).xy;
    vTextureCoordinate = aTextureCoordinate;
}
