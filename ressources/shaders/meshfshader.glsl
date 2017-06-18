#version 330

in vec2 fragST;

uniform sampler2D texture;

out vec4 fragColor;

void main()
{
    fragColor = texture2D(texture,fragST);
}
