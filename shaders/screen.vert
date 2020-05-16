#version 100
attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 UV;

void main()
{
    UV = aTexCoord;
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0); 
} 
