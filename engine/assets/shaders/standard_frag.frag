#version 330 core

in vec2 TexCoords;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform int hasTexture;
uniform vec4 textColor;

void main()
{
    // Always sample the texture, but only use it if hasTexture is 1
    vec4 texColor = texture(textureSampler, TexCoords);
    vec4 baseColor = mix(Color, texColor * Color, float(hasTexture));
    FragColor = baseColor * textColor;
}

