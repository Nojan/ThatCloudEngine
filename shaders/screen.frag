#version 100
precision highp float;

varying vec2 UV;

uniform sampler2D textureSampler;

float decodeFloat(vec4 color) 
{
	const vec4 bitShift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
	return dot(color, bitShift);
}

void main()
{
	float depth = decodeFloat(texture2D(textureSampler, UV));
	depth = pow(depth, 4.0);
	gl_FragColor = vec4(vec3(depth), 1.0);
} 
