#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 v_TexCoords;

uniform mat4 u_ViewProjection;

void main() {
    v_TexCoords = aPos;
    // Multiply aPos by 50.0 to make the cube huge
    vec4 pos = u_ViewProjection * vec4(aPos * 50.0, 1.0); 
    gl_Position = pos.xyww; 
}

#shader fragment
#version 330 core
out vec4 FragColor;
in vec3 v_TexCoords;

uniform samplerCube u_Skybox; // Must be samplerCube for LoadCubeMap

void main() {    
    // v_TexCoords must be the 3D position of the cube vertex
    FragColor = texture(u_Skybox, v_TexCoords); 
}