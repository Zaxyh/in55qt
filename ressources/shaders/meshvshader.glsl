#version 330

uniform mat4 mvp;

in vec3 position;
in vec2 vST;

out vec2 fragST;


void main()
{
    // Calculate vertex position in screen space
    gl_Position = mvp * vec4( position, 1.0 );

    fragST = vST.st;
}
