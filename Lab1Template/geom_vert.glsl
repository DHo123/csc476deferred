#version 120
attribute vec4 vertPos; // in object space
attribute vec3 vertNor; // in object space
attribute vec2 aTexCoord;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
//uniform float lighton;

//Switch toggle for coloring
uniform int terrainToggle;

//GUI toggle
uniform int flag;

varying vec3 color; // Pass to fragment shader
varying vec4 pos;
varying vec3 normal;	
varying vec2 vTexCoord;
//varying float light;

void main()
{
//	if (lighton == 0.0) {
		vec4 norm = (uViewMatrix * uModelMatrix) * vec4(vertNor, 0.0);
		normal = norm.xyz;

		pos = uViewMatrix * uModelMatrix * vertPos;
		gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * vertPos;
	   
		if(flag == 1)
		{
			gl_Position = uModelMatrix * vertPos;
			vTexCoord = aTexCoord;
		}

		if (terrainToggle == 1)
		{
			vTexCoord = aTexCoord;
		}
/*	}
	else {
		//vTexCoord = aTexCoord/vec2(1024/768);

		pos = uViewMatrix * uModelMatrix * vertPos;
		gl_Position = pos;//uProjMatrix * uViewMatrix * uModelMatrix * vertPos;
                gl_Position = gl_Vertex;
                gl_TexCoord[0] = gl_Vertex * 2.0 - 1.0;
	}
	light = lighton;*/
}
