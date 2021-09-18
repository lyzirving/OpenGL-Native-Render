attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;
uniform mat4 uMatrix;
varying vec2 vTextureCoordinate;

void main() {
    gl_Position = uMatrix * vec4(aVertexPos, 1.0);
    vTextureCoordinate = aTextureCoordinate;
}
