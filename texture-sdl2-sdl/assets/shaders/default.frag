#version 150

out vec4 outColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main() {
    outColor = texture(ourTexture, TexCoord);
}
