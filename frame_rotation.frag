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

struct Texture{ //Materijal objekta
	sampler2D kD;
	vec3 kS;
	float shine; //Uglancanost
};

in vec3 chNor;
in vec3 chFragPos;

uniform vec3 uViewPos;

#define NR_POINT_LIGHTS 2
uniform SpotLight spotLight[NR_POINT_LIGHTS];
uniform PointLight pointLight;

out vec4 outCol;
in vec2 TexCoords;
uniform vec3 objectColor;
uniform int uvOn;
uniform int onlyUv;
uniform Texture uTexture;
uniform int isTexture;
uniform int isPointLightOn;

vec3 CalcPointLightMaterial(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightMaterial(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightTexture(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcPointLightTexture(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightTextureForRotation(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(chNor);
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 result;
    if (onlyUv == 1){
        if (uvOn==1)
            result = CalcSpotLightTexture(spotLight[1], norm, chFragPos, viewDir);
        else 
            result = vec3(0.0, 0.0, 0.0);
    } else {
        if (isTexture == 0){
            //if (uvOn==0)
              //  result = vec3(0.0, 0.0, 0.0);
            //else
            result = CalcSpotLightMaterial(spotLight[0], norm, chFragPos, viewDir);
            //vec3 result;
            if (uvOn==1)
                result += CalcSpotLightMaterial(spotLight[1], norm, chFragPos, viewDir);
            if (isPointLightOn==1)
                result +=  CalcPointLightMaterial(pointLight, norm, chFragPos, viewDir);
        } else {
            if (uvOn==0)
                result = vec3(0.0, 0.0, 0.0);
            //else
            if (uvOn==0 && isPointLightOn==0)
                result = CalcSpotLightTextureForRotation(spotLight[0], norm, chFragPos, viewDir); 
                //result = vec3(1.0, 1.0, 1.0);
            //vec3 result;
            if (uvOn==1)
                result += CalcSpotLightTexture(spotLight[1], norm, chFragPos, viewDir);
            if (isPointLightOn==1)
                result +=  CalcPointLightTexture(pointLight, norm, chFragPos, viewDir);
        }
        //result += CalcSpotLight(spotLight[1], norm, chFragPos, viewDir); 
    }
    outCol = vec4(result, 1.0);
}

vec3 CalcSpotLightMaterial(SpotLight light, vec3 normal, vec3 chFragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    //vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient =  0.1 * light.kA;
    vec3 diffuse = light.kD * diff;
    vec3 specular = light.kS * spec * 0.5;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular) * objectColor;
}

vec3 CalcSpotLightTextureForRotation(SpotLight light, vec3 normal, vec3 chFragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.pos - chFragPos);
    //vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = vec3(0.05f, 0.05f, 0.05f) * texture(uTexture.kD, TexCoords).rgb;
    //vec3 ambient = light.kA * (1.0, 1.0, 1.0);
    vec3 diffuse = vec3( 1.0f, 1.0f, 1.0f) * (diff * texture(uTexture.kD, TexCoords).rgb);
    //vec3 diffuse = light.kD * diff * (1.0, 1.0, 1.0);
    vec3 specular = vec3(1.0f, 1.0f, 1.0f) * (spec * (0.5, 0.5, 0.5));
    //vec3 specular = vec3(0.2, 0.2, 0.2);
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    //return (ambient + diffuse + specular);
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLightTexture(SpotLight light, vec3 normal, vec3 chFragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    //vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    //vec3 ambient = light.kA * (1.0, 1.0, 1.0);
    vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    //vec3 diffuse = light.kD * diff * (1.0, 1.0, 1.0);
    vec3 specular = light.kS * spec * 0.2;
    //vec3 specular = vec3(0.2, 0.2, 0.2);
    ambient *= 2.0;
    diffuse *= 2.0;
    specular *= 2.0;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    //return (ambient + diffuse + specular);
    return (ambient + diffuse + specular);
}

vec3 CalcPointLightMaterial(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - chFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient =  0.1 * light.kA;
    vec3 diffuse = diff * light.kD;
    vec3 specular = light.kS * spec * 0.5;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular) * objectColor;
}

vec3 CalcPointLightTexture(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.pos - chFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.pos - chFragPos);
    float attenuation = 1.0 / ((light.constant + light.linear * distance + light.quadratic * (distance * distance)));    
    // combine results
    vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    //vec3 ambient = light.kA * (0.0, 0.0, 1.0);
    vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    //vec3 diffuse = light.kD * diff * (0.0, 0.0, 1.0);
    vec3 specular = light.kS * spec * 0.5;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular) * objectColor;
}