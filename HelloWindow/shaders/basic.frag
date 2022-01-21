#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform mat3 normalMatrix;

//spot light
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform float cutOff;
uniform float outerCutOff;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//control 
uniform bool isTransparent;
uniform bool showShadow;
uniform bool showFog;
uniform bool nightModeEnabled;
uniform bool showSpotLight;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

//shadows
uniform sampler2D shadowMap;
in vec4 fragPosLightSpace;

vec4 fPosEye;
vec3 color;

//positonal light characteristics
vec3 pointLightPos = vec3(-2.5f, 0.3f, 4.1f);
vec3 yellowishColor = vec3(0.5,0.3,0.0);
vec3 whiteColor = vec3(1,1,1);

void computeDirLight()
{
	
    //compute eye space coordinates
    fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * -fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    if(isTransparent) {
		diffuse = max(abs(dot(normalEye, lightDirN)), 0.0f) * lightColor; //abs helps with transparency
	}
	else {
		diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	}

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
	
}

vec3 computePostionalLight() {
	
    vec3 fPositionWorld = vec3(model * vec4(fPosition, 1.0));
	vec3 pointLightPosToFragDir = normalize(pointLightPos - fPositionWorld); // dir from spot light pos to fragment
	
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec3 normalEye = normalize(normalMatrix * -fNormal);
	vec3 lightDirN = vec3(normalize(view * vec4(pointLightPosToFragDir, 0.0f)));
	vec3 viewDir = normalize(- fPosEye.xyz); 

	
    //ambient
	vec3 ambient = ambientStrength * yellowishColor;
    // diffuse shading
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * yellowishColor;
    // specular shading
    vec3 reflectDir = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * specCoeff * yellowishColor;

	
    // attenuation
    float distance = length(pointLightPos - fPositionWorld);
    float attenuation = 1.0 / (1.0f + 0.9 * distance + 0.32 * (distance * distance));   
	
	ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
	
	vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    return color;
}

vec3 computeSpotLight() {
   
	vec3 fPositionWorld = vec3(model * vec4(fPosition, 1.0));
	vec3 spotLightPosToFragDir = normalize(spotLightPos - fPositionWorld); // dir from spot light pos to fragment
	
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec3 normalEye = normalize(normalMatrix * -fNormal);
	vec3 lightDirN = vec3(normalize(view * vec4(spotLightDir, 0.0f)));
	vec3 viewDir = normalize(- fPosEye.xyz); 

	
    //ambient
	vec3 ambient = ambientStrength * whiteColor;
    // diffuse shading
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * whiteColor;
    // specular shading
    vec3 reflectDir = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * specCoeff * whiteColor;

	
    // attenuation
    float distance = length(spotLightPos - fPositionWorld);
    float attenuation = 1.0 / (1.0f + 0.09 * distance + 0.032 * (distance * distance));   
	
    // spotlight intensity
    float theta = dot(normalize(-spotLightDir), spotLightPosToFragDir); //with minus??
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
	
	ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
	
	vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    return color;
}

float computeShadow() {
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	
	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	
	// Check whether current frag pos is in shadow
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
	
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	
	return shadow;
}

float computeFog()
{
 float fogDensity = 0.1f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{

	if(isTransparent) {
		if(texture(diffuseTexture, fTexCoords).a < 0.4f) {
			discard;
		}
	}

    computeDirLight();
	
	if(showShadow && (!nightModeEnabled)) {
		float shadow = computeShadow();
		//compute final vertex color
		color = min((ambient + (1.0f - shadow)*diffuse) * texture(diffuseTexture, fTexCoords).rgb + ((1.0f - shadow)*specular) * texture(specularTexture, fTexCoords).rgb, 1.0f);
	}  else {
		color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
	}
	
	if(nightModeEnabled) { //when in night mode, show light in cottage
		vec3 colorResultFromPoint = computePostionalLight();
		color += colorResultFromPoint;
	}
	
	if(showSpotLight) {
		vec3 colorResultFromSpot = computeSpotLight();
		color += colorResultFromSpot;
	}

	
	if(showFog) {
		float fogFactor = computeFog();
	    vec4 fogColor = vec4(0.8f, 0.8f, 1.0f, 1.0f);
	    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
	} else {
		fColor = vec4(color, 1.0f);
	}

	//fColor = fogColor * (1 – fogFactor) + vec4(color, 1.0f) * fogFactor;
}
