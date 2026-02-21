#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos; 

layout (std140) uniform CameraData {
    mat4 u_ViewProjection;
};

uniform mat4 u_Model;

void main() {
     v_TexCoords = aTexCoords;
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    v_FragPos = vec3(worldPos);
    
    // Normal matrix to handle non-uniform scaling
    v_Normal = mat3(transpose(inverse(u_Model))) * aNormal; 
    
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

   // --- Fresnel Effect ---
    // dot(norm, viewDir) is 1.0 when looking straight at a face, 0.0 at the edges.
    // We invert it (1.0 - dot) so the edges get the "glow".
    float fresnelBias = 0.1; // Base glow
    float fresnelScale = 1.0; // Intensity
    float fresnelPower = 4.0; // How "thin" the rim is
    
    float fresnel = fresnelBias + fresnelScale * pow(1.0 - max(dot(norm, viewDir), 0.0), fresnelPower);
    vec3 fresnelColor = vec3(0.0, 0.5, 1.0); // Let's make it a cool blue rim

    vec4 texColor = texture(u_Texture, v_TexCoords);
    
    // Discard transparent pixels (optional but good for some textures)
    if(texColor.a < 0.1) discard;

    vec3 baseLighting = (ambient + diffuse) * texColor.rgb;
    vec3 finalOutput = baseLighting + (fresnel * fresnelColor);

    FragColor = vec4(finalOutput, texColor.a);
}
