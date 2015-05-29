#version 120

attribute vec4 vertPos; // in object space
uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

varying vec4 pos;

void main() {
	pos = /*uProjMatrix * uViewMatrix * uModelMatrix */ vertPos;
	gl_Position = pos;
}
