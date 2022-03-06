varying vec4 FragColor;
attribute vec3 texCoords;
uniform samplerCube skybox;

void main() {
    FragColor = textureCube(skybox,texCoords);
}