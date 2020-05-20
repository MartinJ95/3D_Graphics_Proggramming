#version 330

// TODO: add uniforms to take in the matrix
uniform mat4 view_projection_xform;
uniform mat4 model_xform;

// TODO: add in variables for each of the streamed attributes
in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texcoord;

out vec2 texcoord;
out vec3 worldPosition;
out vec3 worldNormal;


// TODO: specify out variables to be varied to the FS

void main(void)
{
	worldPosition = (model_xform * vec4(vertex_position, 1.0)).xyz;
	worldNormal = (model_xform * vec4(vertex_normal, 0.0)).xyz;
	texcoord = vertex_texcoord;
	gl_Position = view_projection_xform * model_xform * vec4(vertex_position, 1.0);
	//gl_Position = vec4(vertex_position, 1.0);
}
