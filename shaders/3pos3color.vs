#version 330 core  
layout (location = 0) in vec3 aPos;    // pos variable has location 0
layout(location = 1) in vec3 aColor;   // color variable has location 1

out vec3 ourColor;  
uniform float offset;
out vec3 ourPosition;

uniform mat4 transform;

void main()  
{  
	gl_Position = transform * vec4(aPos.x + offset, aPos.y, aPos.z, 1.0);  
	ourColor = aColor; 
	ourPosition = vec3(gl_Position.x, gl_Position.y, gl_Position.z);
};
