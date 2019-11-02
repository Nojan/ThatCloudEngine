#version 100

attribute vec3 vertexPosition_modelspace;
attribute vec2 textureCoord;
attribute float a_alpha;
attribute float a_textureIndex;

varying vec2 UV;
varying float alpha;
varying float v_textureIndex;

uniform mat4 mvp;

void main() {
    UV = textureCoord;
    alpha = a_alpha;
    v_textureIndex = a_textureIndex;
    gl_Position =  mvp * vec4(vertexPosition_modelspace, 1);
}

