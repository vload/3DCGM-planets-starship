#version 410

layout(location = 0) in vec3 position;

out vec3 vsPosition;

void main()
{
    vsPosition = position;
}
