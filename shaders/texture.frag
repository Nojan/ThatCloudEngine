#version 100
precision highp float;
// Interpolated values from the vertex shaders
varying vec2 UV;
varying vec3 vertexNormalMS;
varying vec3 positionMS;
varying vec4 shadowCoord;

uniform sampler2D textureSampler;
uniform sampler2D shadowMap;
uniform vec3 lightPositionMS;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;

float decodeFloat(vec4 color) 
{
	const vec4 bitShift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
	return dot(color, bitShift);
}

float shadow()
{
    float texelSize = 1.0 / 1024.0;
    const float minBias = 0.005;
    const float maxBias = 0.05;
    float bias = minBias; //max(maxBias * (1.0 - dot(vertexNormalMS, lightPositionMS)), minBias);
    float visibility = 1.0;
    vec3 shadow = shadowCoord.xyz / shadowCoord.w;
    int shadowHit = 1;
    for(int i = -1; i <= 1; ++i)
    {
        for(int j = -1; j <= 1; ++j)
        {
            float depth = decodeFloat( texture2D(shadowMap, shadow.xy + vec2(i, j) * texelSize) );
            if ( depth < shadow.z - bias)
            {
                shadowHit += 1;
            }
        }
    }

    visibility /= float(shadowHit);
    return visibility;
}

vec4 blinn_phong(const in vec3 fragNormalMS, const in vec3 lightPositionMS, const in vec3 positionMS, const in float visibility) 
{
    vec4 diffuseColor = lightDiffuse * vec4(vec3(visibility), 1.0);
    vec4 ambient = vec4(vec3(0.1), 1.0);
    vec4 specularColor = lightSpecular * vec4(vec3(visibility), 1.0);
    float shininess = 32.0;
    vec3 lightDir = normalize(lightPositionMS - positionMS);
    float diffuseIntensity = max(dot(fragNormalMS, lightDir), 0.0);
    vec4 diffuse = diffuseIntensity * diffuseColor;
    float specularIntensity = 0.0;
    if (0.0 < diffuseIntensity) {
        vec3 h = normalize(fragNormalMS + lightPositionMS);
        float intSpec = max(dot(h, fragNormalMS), 0.0);
        specularIntensity = pow(intSpec, shininess);
    }
    vec4 specular = specularIntensity * specularColor;
    return ambient + diffuse + specular;
}

void main() {
    float visibility = shadow();

    vec4 color = blinn_phong(normalize(vertexNormalMS), lightPositionMS, positionMS, visibility);
    color = color * texture2D(textureSampler , fract(vec2(UV.x, UV.y)) );
    gl_FragColor = vec4(color.rgb, 1.0);
}
