#version 330 core  
layout (location = 0) in vec3 aPos;    // pos variable has location 0
layout(location = 1) in vec3 aColor;   // color variable has location 1
out vec3 ourColor;  
void main()  
{  
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);  
	ourColor = aColor;  
};
