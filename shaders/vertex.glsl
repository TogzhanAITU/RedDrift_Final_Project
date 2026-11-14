#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 ourColor;

void main()
{
    gl_Position = vec4(aPos.x / 640.0 - 1.0, 1.0 - aPos.y / 360.0, 0.0, 1.0);
    ourColor = aColor;
}
