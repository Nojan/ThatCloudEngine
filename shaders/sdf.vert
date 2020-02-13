#version 300 es
precision highp float;

in vec2 vPosition;

void main()
{
    gl_Position =  vec4(vPosition, 0, 1);
}

