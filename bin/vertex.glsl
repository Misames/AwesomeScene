// Model 3D
attribute vec4 a_position;
attribute vec3 a_normal;

// Matrice de modification
uniform float u_time;
uniform mat4 u_rotationMatrix;
uniform mat4 u_translationMatrix;
uniform mat4 u_projectionMatrix;

uniform mat4 u_view;

varying vec3 v_normal;

// Texture
attribute vec2 a_texcoords;
varying vec2 v_texcoords;

void main(void)
{
    v_texcoords = a_texcoords;
    mat4 worldMatrix = u_translationMatrix * u_rotationMatrix;
    v_normal = mat3(worldMatrix) * a_normal;
    // gl_Position = u_projectionMatrix * worldMatrix * a_position;
    gl_Position = u_projectionMatrix * a_position;
}