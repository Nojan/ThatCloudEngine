#version 100
precision highp float;

varying vec2 UV;

uniform sampler2D textureSampler;

void main()
{
    vec3 col = texture2D(textureSampler, UV).rgb;
    gl_FragColor = vec4(col, 1.0);
} 
