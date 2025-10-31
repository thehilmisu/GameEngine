#version 330 core

in vec2 TexCoords;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D texture1;
uniform bool useTexture;

void main()
{
    if (useTexture) {
        FragColor = texture(texture1, TexCoords) * Color;
    } else {
        FragColor = Color;
    }
}

