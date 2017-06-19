#version 450

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

in vec3 position;
in vec2 vST;
in vec3 normal;

out vec2 fragST;
out vec4 fragPosWorld;
out vec4 fragNorWorld;

void main()
{
    // Calculate vertex position in screen space
    gl_Position = p * v * m * vec4( position, 1.0 );    

    fragPosWorld = m * vec4(position, 1);
    fragNorWorld = m * vec4(normal, 0);
    fragST = vST;
}
