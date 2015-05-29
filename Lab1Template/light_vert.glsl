#version 120
attribute vec4 vertPos; // in object space
//attribute vec3 vertNor; // in object space
//attribute vec2 aTexCoord;

//Switch toggle for coloring
uniform int terrainToggle;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

varying vec2 vTexCoord;
varying vec4 pos;
void main()
{
//	gl_TexCoord[0] = vertPos;
	gl_TexCoord[0] = gl_Vertex;
//	gl_Position = gl_Vertex * 2.0 - 1.0;
	pos = uProjMatrix *  uViewMatrix * uModelMatrix * vertPos;
	gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * vertPos;
//	gl_Position = vertPos * 2.0 - 1.0;

//  gl_FrontColor = vec4(0.5, 1.0, 0.5, 1.0);

//	if (terrainToggle == 1)
//	{
//		vTexCoord = aTexCoord;
//	}
}
