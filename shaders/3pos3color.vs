#version 330 core  
layout (location = 0) in vec3 aPos;    // pos variable has location 0
layout(location = 1) in vec2 aColor;   // color variable has location 1

out vec3 ourColor;  
uniform float offset;
out vec3 ourPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()  
{  
	gl_Position = projection * view * model * vec4(aPos, 1.0);  
	ourColor = vec3(aColor.x,0.3,aColor.y); 
	ourPosition = vec3(gl_Position.x, gl_Position.y, gl_Position.z);
};
