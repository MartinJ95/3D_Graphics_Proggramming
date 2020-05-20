#version 330

struct Material
{
	vec3 ambient_colour;
	vec3 diffuse_colour;
	vec3 specular_colour;
	bool isShiny;
	float shininess;
};

struct Light
{
	vec3 position;
	vec3 intensity;
	float range;
};

struct SpotLight
{
	vec3 position;
	vec3 intensity;
	vec3 direction;
	float field_of_view;
	float range;
	bool isOn;
};

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 TBN;

uniform Material material;
uniform SpotLight torch;
uniform Light lights[22];
uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform vec3 ambient_lighting;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalMap;
uniform sampler2D bumpMap;
uniform bool hasNormalMap;

float height_scale = 0.1f;

vec3 surfaceDiffuse;
vec3 surfaceSpecular;
vec3 finalColour = vec3(0, 0, 0);
vec3 normal;
vec2 texCoords;

vec2 parallaxMapping()
{
	float height = texture(bumpMap, frag_texcoord).r;
	vec2 p = camera_dir.xy / camera_dir.z * (height * height_scale);
	return frag_texcoord - p;
}

vec3 getLightDiffuse(Light light)
{
	float distance = length(light.position - frag_position);
	float attenuation = smoothstep(light.range, light.range / 2, distance);
	vec3 L = normalize(light.position - frag_position);
	float diffuse_intensity = max(0, dot(L, normal));
	return (diffuse_intensity * surfaceDiffuse) * attenuation;
}

vec3 getLightSpecular(Light light)
{
	float distance = length(light.position - frag_position);
	float attenuation = smoothstep(light.range, light.range / 2, distance);
	vec3 L = normalize(light.position - frag_position);
	vec3 viewDir = normalize(camera_pos - frag_position);
	vec3 reflectDir = reflect(-L, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	return spec * surfaceSpecular * attenuation;
}

vec3 getSpotlightDiffuse(SpotLight light)
{
	float distance = length(light.position - frag_position);
	float attenuation = smoothstep(light.range, light.range / 2, distance);
	vec3 L = normalize(light.position - frag_position);
	vec3 s = -L;
	float fc = smoothstep(cos(0.5 * radians(light.field_of_view)), 1, dot(s, light.direction));
	float diffuse_intensity = max(0, dot(L, normal));
	return (diffuse_intensity * surfaceDiffuse) * attenuation * fc;
}

vec3 getLightSpecular(SpotLight light)
{
	float distance = length(light.position - frag_position);
	float attenuation = smoothstep(light.range, light.range / 2, distance);
	vec3 L = normalize(light.position - frag_position);
	vec3 viewDir = normalize(camera_pos - frag_position);
	vec3 reflectDir = reflect(-L, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	return spec * surfaceSpecular * attenuation;
}


out vec4 fragment_colour;

void main(void)
{
	
	

	if (hasNormalMap)
	{
		normal = texture(normalMap, frag_texcoord).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = normalize(TBN * normal);
		texCoords = parallaxMapping();
		if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
			discard;
	}
	else
	{
		normal = normalize(frag_normal);
		texCoords = frag_texcoord;
	}

	surfaceDiffuse = (vec4(material.diffuse_colour, 1.0) * texture(diffuseTex, texCoords)).xyz;
	surfaceSpecular = (vec4(material.specular_colour, 1.0) * texture(specularTex, texCoords)).xyz;

	if (material.isShiny)
	{
		for (int i = 0; i < 22; i++)
		{
			vec3 diffuse = getLightDiffuse(lights[i]);
			vec3 specular = getLightSpecular(lights[i]);
			finalColour += lights[i].intensity * (diffuse + specular);
		}
	}
	else
	{
		for (int i = 0; i < 22; i++)
		{
			finalColour += lights[i].intensity * (getLightDiffuse(lights[i]));
		}
	}
	if (torch.isOn)
	{
		if (material.isShiny)
		{
			vec3 diffuse = getSpotlightDiffuse(torch);
			vec3 specular = getLightSpecular(torch);
			finalColour += torch.intensity * (diffuse + specular);
		}
		else
		{
			finalColour += torch.intensity * (getSpotlightDiffuse(torch));
		}
	}

	vec3 reflectedLight = (ambient_lighting * material.ambient_colour) + finalColour;

	fragment_colour = vec4(reflectedLight, 1.0);
	//fragment_colour = vec4(1.0, 0.33, 0.0, 1.0);
}
