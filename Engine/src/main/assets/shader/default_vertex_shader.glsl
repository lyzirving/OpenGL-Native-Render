attribute vec3 aVertexPos;
attribute vec2 aTextureCoordinate;
varying vec2 vTextureCoordinate;

void main() {
    gl_Position = vec4(aVertexPos,1.0);
    vTextureCoordinate = aTextureCoordinate;
}
