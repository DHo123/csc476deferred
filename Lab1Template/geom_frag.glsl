#version 120

varying vec3 normal;	
varying vec2 vTexCoord;

//varying float light;
uniform sampler2D uTexUnit;
uniform vec3 kd;
uniform vec3 ks;
uniform vec3 ka;
uniform float s;
uniform int flag;

//Switch toggle for coloring
uniform int terrainToggle;

void main () {
	gl_FragData[0] = vec4(normal, 1); //normal
	//gl_FragData[1] = texture2D(diffuse_texture, vTexCoord);      //diffuse
	
		gl_FragData[1] = texture2D(uTexUnit, vTexCoord);
	if (terrainToggle == 1 || flag == 1)
	{
		gl_FragData[1] *= 1; //texture2D(uTexUnit, vTexCoord);
		//gl_FragColor = texColor1 * 2.0 * vec4(color.r, color.g, color.b, 1.0);
	}
	else
	{
		//gl_FragColor = vec4(color.r, color.g, color.b, 1.0);
		gl_FragData[1] *= vec4(0.0, 0.0, 0.0, 1.0);
	}
}
