#version 330

struct Light
{
	vec3 position;
	vec3 intensity;
	float range;
};

struct spotLight
{
	vec3 position;
	vec3 intensity;
	vec3 direction;
	float field_of_view;
	float range;
};

struct Material
{
	vec3 ambientCol;
	vec3 diffuseCol;
	sampler2D diffuseTex;
	bool diffuseIsTex;
	float shininess;
	vec3 specularCol;
	sampler2D specularTex;
	bool specularIsTex;
	bool isShiny;
};

uniform mat4 view_projection_xform;
uniform mat4 model_xform;
uniform Material material;
uniform Light lights[22];
uniform vec3 ambientLight;
uniform vec3 cameraPosition;
uniform spotLight spotlight;
uniform bool spotLightActive;

in vec2 texcoord;
in vec3 worldPosition;
in vec3 worldNormal;

vec3 normal;
vec3 surfaceDiffuse;
vec3 surfaceSpecular;

out vec4 fragment_colour;

vec3 getLightDiffuse(Light light, vec3 surfaceDiffuse, vec3 worldPosition, vec3 vertexNormal)
{
	float distance = length(light.position - worldPosition);
	float attenuation = smoothstep(light.range, light.range/2, distance);
	vec3 L = normalize(light.position - worldPosition);
	float diffuse_intensity = max(0, dot(L, vertexNormal));
	return (diffuse_intensity * surfaceDiffuse) * attenuation;
}

vec3 getSpotLightDiffuse(spotLight light, vec3 surfaceDiffuse, vec3 worldPosition, vec3 vertexNormal)
{
	float distance = length(light.position - worldPosition);
	float attenuation = smoothstep(light.range, light.range/2, distance);
	vec3 L = normalize(light.position - worldPosition);
	vec3 s = -L;
	float fc = smoothstep(cos(0.5 * radians(light.field_of_view)), 1, dot(s, light.direction));
	float diffuse_intensity = max(0, dot(L, vertexNormal));
	return (diffuse_intensity * surfaceDiffuse) * attenuation * fc;
}

vec3 getLightSpecular(Light light, vec3 surfaceSpecular, float shininess, vec3 cameraPosition, vec3 worldPosition, vec3 vertexNormal)
{
	float distance = length(light.position - worldPosition);
	float attenuation = smoothstep(light.range, light.range/2, distance);
	vec3 L = normalize(light.position - worldPosition);
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-L, vertexNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	return spec * surfaceSpecular * attenuation;
}

vec3 getSpotLightSpecular(spotLight light, vec3 surfaceSpecular, float shininess, vec3 cameraPosition, vec3 worldPosition, vec3 vertexNormal)
{
	float distance = length(light.position - worldPosition);
	float attenuation = smoothstep(light.range, 0, distance);
	vec3 L = normalize(light.position - worldPosition);
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-L, vertexNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	return spec * surfaceSpecular * attenuation;
}

void main(void)
{
	if (material.diffuseIsTex)
	{
		surfaceDiffuse = (vec4(material.diffuseCol, 1.0) * texture(material.diffuseTex, texcoord)).xyz;
	}
	else
	{
		surfaceDiffuse = material.diffuseCol;
	}
	if (material.specularIsTex)
	{
		surfaceSpecular = (vec4(material.specularCol, 1.0) * texture(material.specularTex, texcoord)).xyz;
	}
	else
	{
		surfaceSpecular = material.specularCol;
	}
	normal = normalize(worldNormal);
	vec3 finalColor = vec3(0, 0, 0);
	if (material.isShiny)
	{
		for (int i = 0; i < 12; i++)
		{
			vec3 diffuse =  getLightDiffuse(lights[i], surfaceDiffuse, worldPosition, normal);
			vec3 specular = getLightSpecular(lights[i], surfaceSpecular, material.shininess, cameraPosition, worldPosition, normal);
			finalColor += lights[i].intensity * (diffuse + specular);
		}
		if (spotLightActive)
		{
			vec3 diffuse = getSpotLightDiffuse(spotlight, surfaceDiffuse, worldPosition, normal);
			vec3 specular = getSpotLightSpecular(spotlight, surfaceSpecular, material.shininess, cameraPosition, worldPosition, normal);
			finalColor += spotlight.intensity * (diffuse + specular);
		}
	}
	else
	{
		for (int i = 0; i < 12; i++)
		{
			vec3 diffuse = lights[i].intensity * getLightDiffuse(lights[i], surfaceDiffuse, worldPosition, normal);
			finalColor += (diffuse);
		}
		if (spotLightActive)
		{
			vec3 diffuse = spotlight.intensity * getSpotLightDiffuse(spotlight, surfaceDiffuse, worldPosition, normal);
			finalColor += (diffuse);
		}
	}

	

	//vec3 reflected_light = surfaceDiffuse * (ambientLight + finalColor);
	vec3 reflected_light = (ambientLight * material.ambientCol) + finalColor;
	fragment_colour = vec4(reflected_light, 1.0);

	
	/*if (material.diffuseIsTex)
	{
		fragment_colour = texture(material.diffuseTex, texcoord);
	}
	else
	{
		fragment_colour = vec4(material.diffuseCol, 1.0);
	}*/
}
