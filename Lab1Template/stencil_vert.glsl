#version 120

attribute vec4 vertPos; // in object space
uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

void main() {
	gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * vertPos;
}
