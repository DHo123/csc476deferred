#version 120

varying vec4 pos;
varying vec2 vTexCoord;

uniform sampler2D normal_texture, diffuse_texture;


void main()
{
	vec2 TexCoord = gl_FragCoord.xy / vec2(1024, 768);
	vec3 dif = texture2D(diffuse_texture, TexCoord).xyz;
	vec3 norm = texture2D(normal_texture, TexCoord).xyz;
	norm = normalize(norm);

	gl_FragColor = vec4(dif, 1.0);

	// Light calculations here
	vec3 l = vec3(0.0);

	vec3 ld = vec3(1.0, 1.0, 1.0) - vec3(pos);
	float ld2 = dot(ld, ld);
	float ldis = sqrt(ld2);
	ld /= ldis;
	float ndotld = max(dot(norm, ld), 0.0);
	l += (vec3(0.5, 1.0, 0.5) + vec3(1.0, 0.5, 0.5) * ndotld)/ 0.5;

	gl_FragColor.rgb *= l;

	//gl_FragColor = vec4(0.5, 0.0, 1.0, 1.0);
	//gl_FragColor = texture2D(diffuse_texture, vTexCoord);
//	gl_FragColor = texture2D(diffuse_texture, gl_TexCoord[0].st);
	//gl_FragData[2] = vec4(0.5, 0.5, 0.5, 1.0);
	//gl_FragData[0] *= vec4(normalize(texture2D(normal_texture, gl_TexCoord[0].st).rgb),1.0);
/*
    vec3 Normal = normalize(texture2D(normal_texture, vTexCoord).rgb);
    vec3 l = vec3(0.0);

    vec3 ld = vec3(1.0, 1.0, 1.0) - vec3(pos);
    float ld2 = dot(ld, ld);
    float ldis = sqrt(ld2);
    ld /= ldis;
    float ndotld = max(dot(Normal, ld), 0.0);
    l += (vec3(0.5, 1.0, 0.5) + vec3(1.0, 0.5, 0.5) * ndotld)/ 0.5;

    gl_FragColor.rgb *= l;
*/
}
