#version 100
#define numTextures 8

precision highp float;
varying vec2 UV;
varying float textureIndex;
varying float alpha;

uniform sampler2D textureSampler[numTextures];

void main() {
    vec4 alpha4 = vec4(1, 1, 1, alpha);
    vec4 color = texture2D( textureSampler[int(textureIndex + 0.5)], fract(UV) );
    if( color.w < 0.01 )
        discard; 
    gl_FragColor = color * alpha4;
}
