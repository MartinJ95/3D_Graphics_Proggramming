#version 330

// TODO: add uniforms to take in the matrix
uniform mat4 view_projection_xform;
uniform mat4 model_xform;

// TODO: add in variables for each of the streamed attributes

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texcoord;
in vec3 vertex_tangent;

// TODO: specify out variables to be varied to the FS

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;
out mat3 TBN;

void main(void)
{
	mat3 normalMatrix = transpose(inverse(mat3(model_xform)));

	vec3 bitangent = cross(vertex_normal, vertex_tangent);
	vec3 T = normalize(normalMatrix * vertex_tangent);
	vec3 N = normalize(normalMatrix * vertex_normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = normalize(normalMatrix * bitangent);

	TBN = mat3(T, B, N);

	frag_position = vec4(model_xform * vec4(vertex_position, 1.0)).xyz;
	//frag_normal = vec4(model_xform * vec4(vertex_normal, 0.0)).xyz;
	//frag_normal = vertex_normal;
	frag_normal = normalMatrix * vertex_normal;
	frag_texcoord = vertex_texcoord;
	 gl_Position = view_projection_xform * model_xform * vec4(vertex_position, 1.0);
}
