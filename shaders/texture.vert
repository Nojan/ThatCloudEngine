#version 100

// Input vertex data, different for all executions of this shader.
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 TexCoord0;

// Output data ; will be interpolated for each fragment.
varying vec2 UV;
varying vec3 vertexNormalMS;
varying vec3 positionMS;
varying vec4 shadowCoord;

// Values that stay constant for the whole mesh.
uniform mat4 mvp;
uniform mat4 model;
uniform mat3 modelNormal;

uniform mat4 depthMVP;

void main(){
    UV = TexCoord0;
    vec4 Position4 = vec4(Position, 1);
    positionMS = vec3(model * Position4);
    vertexNormalMS = modelNormal * Normal;
    gl_Position =  mvp * Position4;

    shadowCoord = depthMVP * Position4;
}

