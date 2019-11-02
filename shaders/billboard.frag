#version 100
#define numTextures 8

precision highp float;
varying vec2 UV;
varying float v_textureIndex;
varying float alpha;

uniform sampler2D textureSampler[8];

vec4 getSampleFromArray(int ndx, vec2 uv) {
    vec4 color = vec4(0);
    for (int i = 0; i < numTextures; ++i) {
        vec4 c = texture2D(textureSampler[i], uv);
        if (i == ndx) {
            color += c;
        }
    }
    return color;
}

void main() {
    vec4 alpha4 = vec4(1, 1, 1, alpha);
    int textureIndex = int(v_textureIndex + 0.5);
    vec4 color = getSampleFromArray(textureIndex, fract(UV));
    if( color.w < 0.01 )
        discard; 
    gl_FragColor = color * alpha4;
}
