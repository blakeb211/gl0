#version 330 core  
in vec3 ourColor;  
out vec4 FragColor;  
in vec3 ourPosition;
in vec2 TexCoord;


uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
};
