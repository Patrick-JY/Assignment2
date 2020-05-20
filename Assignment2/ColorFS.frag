#version 330 core

// interpolated values from the vertex shaders
in vec3 vN;
in vec3 vL;
in vec3 vE;
in vec3 vR;

// light and material structs
struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

//uniform input data
uniform Light uLight;
uniform Material uMaterial;

// output data
out vec3 fColor;

void main()
{

	
	vec3 N = normalize(vN);
    vec3 L = normalize(vL);
    vec3 E = normalize(vE);
    vec3 R = normalize(vR);

	vec3 ambient  = uLight.ambient * uMaterial.ambient;
    vec3 diffuse  = uLight.diffuse * uMaterial.diffuse * max(dot(L, N), 0.0);
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);

	if(dot(L, N) > 0.0f)
	    specular = uLight.specular * uMaterial.specular * pow(max(dot(E, R), 0.0), uMaterial.shininess);

    

	// set output color
	fColor = diffuse + specular + ambient;
}