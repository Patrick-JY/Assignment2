#version 330 core

// input data (different for all executions of this shader)
in vec3 aPosition;
in vec3 aNormal;

struct Light
{
	vec3 position;	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

// uniform input data
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uViewMatrix;
uniform Light uLight;

// output data (will be interpolated for each fragment)
out vec3 vN;
out vec3 vL;
out vec3 vE;
out vec3 vR;

void main()
{
	// set vertex position
    gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0);

	// eye/camera space
	vec3 EC_VertexPosition = (uModelViewMatrix * vec4(aPosition, 1.0)).xyz;
	vec3 EC_Normal = (uModelViewMatrix * vec4(aNormal, 0.0)).xyz;

	// the color of each vertex will be interpolated
	vN = normalize(EC_Normal);
	vL = normalize((uViewMatrix * vec4(uLight.position, 1.0)).xyz - EC_VertexPosition);
	vE = normalize(-EC_VertexPosition);
	vR = reflect(-vL, vN);
}