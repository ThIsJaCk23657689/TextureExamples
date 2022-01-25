#version 150

out vec4 outColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main() {
    vec4 finalColor = texture(ourTexture, TexCoord);
    if (finalColor.a < 0.1) {
        discard;
    }
    outColor = finalColor;
}
