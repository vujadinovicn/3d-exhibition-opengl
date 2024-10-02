#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor; //Normale
layout(location = 2) in vec2 TexCoords;
out vec3 chFragPos; //Interpolirana pozicija fragmenta
out vec3 chNor; //Interpolirane normale

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

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
	vec3 kS;
	float shine; //Uglancanost
};

uniform Material uMaterial;
uniform Texture uTexture;

uniform vec3 uViewPos;	

#define NR_POINT_LIGHTS 2
uniform SpotLight spotLight[NR_POINT_LIGHTS];
uniform PointLight pointLight;
uniform int uvOn;
uniform int onlyUv;
uniform int isPointLightOn;
uniform int isTexture;
uniform vec3 objectColor;
uniform vec2 uPos;

out vec2 chTex; 
out vec3 fragColor;

vec3 CalcPointLightMaterial(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightMaterial(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightTexture(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcPointLightTexture(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLightTextureBoost(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 
vec3 CalcSpotLightTextureForRotation(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 

void main()
{
    chTex = TexCoords;
    vec3 t = vec3(inPos.x-uPos.x , inPos.y+uPos.y, inPos.z);
	chFragPos = vec3(uM * vec4(t, 1.0));
	gl_Position = uP * uV * vec4(chFragPos, 1.0);
	chNor = mat3(transpose(inverse(uM))) * inNor;
	vec3 normal = normalize(chNor);
	vec3 viewDir = normalize(uViewPos - chFragPos);

	vec3 color;
    if (onlyUv == 1){
        if (uvOn==1)
            color = CalcSpotLightTextureBoost(spotLight[1], normal, chFragPos, viewDir);
        else 
            color = vec3(0.0, 0.0, 0.0);
    } else{
        if (isTexture == 0){
            if (uvOn==0)
                color = vec3(0.0, 0.0, 0.0);
            else
                color = CalcSpotLightMaterial(spotLight[0], normal, chFragPos, viewDir); 
            //vec3 result;
            if (uvOn==1)
                color += CalcSpotLightMaterial(spotLight[1], normal, chFragPos, viewDir);
            if (isPointLightOn==1)
                color +=  CalcPointLightMaterial(pointLight, normal, chFragPos, viewDir);
        
        } else {
            

            if (uvOn==0)
                color = vec3(0.0, 0.0, 0.0);
            //else
            if (uvOn==0 && isPointLightOn==0)
                color = CalcSpotLightTextureForRotation(spotLight[0], normal, chFragPos, viewDir); 
                //result = vec3(1.0, 1.0, 1.0);
            //vec3 result;
            if (uvOn==1)
                color += CalcSpotLightTexture(spotLight[1], normal, chFragPos, viewDir);
            if (isPointLightOn==1)
                color +=  CalcPointLightTexture(pointLight, normal, chFragPos, viewDir);
        
        }
    }

	fragColor = color;
	
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
    vec3 diffuse = diff * light.kD;
    vec3 specular = light.kS * spec * 0.5;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
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
    //vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    vec3 ambient = light.kA * 1;
    //vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    vec3 diffuse = light.kD * diff;
    vec3 specular = light.kS * spec * 0.5;
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


vec3 CalcSpotLightTextureBoost(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
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
    //vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    vec3 ambient = light.kA * 1;
    //vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    vec3 diffuse = light.kD * diff;
    vec3 specular = light.kS * spec * 0.5;
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
    return (ambient + diffuse + specular);
}

vec3 CalcPointLightTexture(PointLight light, vec3 normal, vec3 chFragPos, vec3 viewDir){
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
    //vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    //vec3 ambient = light.kA * texture(uTexture.kD, TexCoords).rgb;
    vec3 ambient = light.kA * 0.1;
    //vec3 diffuse = light.kD * (diff * texture(uTexture.kD, TexCoords).rgb);
    vec3 diffuse = light.kD * diff;
    vec3 specular = light.kS * spec * 0.5;
    //vec3 diffuse = light.kD * diff * (1.0, 1.0, 1.0);
    //vec3 specular = light.kS * spec * 0.5;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}