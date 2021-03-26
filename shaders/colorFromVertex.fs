#version 330 core  
in vec3 ourColor;  
out vec4 FragColor;  
in vec3 ourPosition;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()  
{  
	FragColor = texture(ourTexture, TexCoord);
};
