#version 330 core

struct SpotLight {
    vec3 pos;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 kA;
    vec3 kD;
    vec3 kS;       
};

struct PointLight {
    vec3 pos;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 kA;
    vec3 kD;
    vec3 kS;
};

struct Material{ //Materijal objekta
	vec3 kA;
	vec3 kD;
	vec3 kS;
	float shine; //Uglancanost
};

struct Texture{ //Materijal objekta
	sampler2D kD;
	sampler2D kS;
	float shine; //Uglancanost
};

in vec3 chNor;
in vec3 chFragPos;

uniform Material uMaterial;
uniform Texture uTexture;
uniform vec3 uViewPos;

#define NR_POINT_LIGHTS 2
uniform SpotLight spotLight[NR_POINT_LIGHTS];
uniform PointLight pointLight;
uniform int uvOn;
uniform int isMaterial;
uniform int isPointLightOn;

in vec2 TexCoords;

uniform int wallOff;
out vec4 outCol;

vec3 CalcPointLightMaterial(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightMaterial(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightTexture(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcPointLightTexture(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(chNor);
    vec3 viewDir = normalize(uViewPos - chFragPos);
    //for(int i = 0; i < NR_POINT_LIGHTS; i++)
    vec3 result;
    if (isMaterial == 1) {
        result = CalcSpotLightMaterial(spotLight[0], norm, chFragPos, viewDir); 
        if (uvOn==1)
            result += CalcSpotLightMaterial(spotLight[1], norm, chFragPos, viewDir); 
        if (isPointLightOn==1)
            result +=  CalcPointLightMaterial(pointLight, norm, chFragPos, viewDir);
    } else {
        result = CalcSpotLightTexture(spotLight[0], norm, chFragPos, viewDir); 
        if (uvOn==1)
            result += CalcSpotLightTexture(spotLight[1], norm, chFragPos, viewDir); 
        if (isPointLightOn==1)
            result +=  CalcPointLightTexture(pointLight, norm, chFragPos, viewDir);
    }
    outCol = vec4(result, 1.0);
}

vec3 CalcSpotLightMaterial(SpotLight light, vec3 normal, vec3 chFragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shine);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.kA * uMaterial.kD;
    vec3 diffuse = light.kD * (diff * uMaterial.kD);
    vec3 specular = light.kS * (spec * uMaterial.kS);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLightTexture(SpotLight light, vec3 normal, vec3 chFragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uTexture.shine);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    vec3 specular = light.kS * (spec * texture(uTexture.kS, TexCoords).rgb);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLightMaterial(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shine);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.kA * uMaterial.kD;
    vec3 diffuse = light.kD * (diff * uMaterial.kD);
    vec3 specular = light.kS * (spec * uMaterial.kS);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLightTexture(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uTexture.shine);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    vec3 specular = light.kS * (spec * texture(uTexture.kS, TexCoords).rgb);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}