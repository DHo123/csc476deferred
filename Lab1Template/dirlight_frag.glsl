#version 120

uniform sampler2D normal_texture, diffuse_texture;
varying vec4 pos;

void main() {
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
}
