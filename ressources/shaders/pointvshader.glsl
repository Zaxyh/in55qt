#version 330

uniform mat4 mvp;
uniform float point_size;

in vec3 position;
in vec3 color;

out vec3 fColor;

//! [0]
void main()
{
    gl_PointSize = point_size;

    // Calculate vertex position in screen space
    gl_Position = mvp * vec4( position, 1.0 );

    fColor = color;
}
//! [0]
