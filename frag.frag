#version 330 core
in vec3 verColor;
out vec4 FragColor;
uniform vec3 colorTweak;

void main()
{
    FragColor = vec4(verColor * colorTweak, 1.0f);
}
