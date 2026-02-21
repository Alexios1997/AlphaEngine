#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
// This matches the glVertexAttribPointer(3, ...) in your Mesh class
layout (location = 3) in mat4 aInstanceMatrix; 

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos; 

layout (std140) uniform CameraData {
    mat4 u_ViewProjection;
};

void main() {
    v_TexCoords = aTexCoords;
    
    // Use aInstanceMatrix instead of u_Model
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    v_FragPos = vec3(worldPos);
    
    // Normal matrix using the instance matrix
    v_Normal = mat3(aInstanceMatrix) * aNormal;
    
    gl_Position = u_ViewProjection * worldPos;
}

#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_FragPos;

uniform sampler2D u_Texture;
uniform vec3 u_LightDir; // Direction TO the light
uniform vec3 u_ViewPos;  // Camera World Position

void main() {
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightDir);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	
    // Ambient
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * vec3(1.0); 

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    float specularStrength = 0.5;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0); 
    vec3 specular = specularStrength * spec * vec3(1.0);

    vec4 texColor = texture(u_Texture, v_TexCoords);
    
    // Discard transparent pixels (optional but good for some textures)
    if(texColor.a < 0.1) discard;

    FragColor = vec4((ambient + diffuse + specular) * texColor.rgb, texColor.a);
}
