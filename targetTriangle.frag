#version 330 core
out vec4 FragColor;
uniform vec3 bunnyColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;
in vec3 fragPos;
in vec3 normal;

void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * bunnyColor;
    FragColor = vec4(result, 1.0);

}
