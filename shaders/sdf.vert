#version 100 
precision highp float;

attribute vec2 vPosition;

void main()
{
    gl_Position =  vec4(vPosition, 0, 1);
}

