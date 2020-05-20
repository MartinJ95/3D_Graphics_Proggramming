#include "MyView.hpp"
#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
//#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const sponza::Context * scene)
{
    scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);

	GLint compile_status = GL_FALSE;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_string
		= tygra::createStringFromFile("resource:///sponza_vs.glsl");
	const char * vertex_shader_code = vertex_shader_string.c_str();
	glShaderSource(vertex_shader, 1,
		(const GLchar **)&vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragment_shader_string
		= tygra::createStringFromFile("resource:///sponza_fs.glsl");
	const char * fragment_shader_code = fragment_shader_string.c_str();
	glShaderSource(fragment_shader, 1,
		(const GLchar **)&fragment_shader_code, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(fragment_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	// Create shader program & shader in variables
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);

	// TODO: glBindAttribLocation for all shader streamed IN variables e.g.
	glBindAttribLocation(shader_program_, kVertexPosition, "vertex_position");
	glBindAttribLocation(shader_program_, kVertexNormal, "vertex_normal");
	glBindAttribLocation(shader_program_, kVertexTexcoord, "vertex_texcoord");

	glDeleteShader(vertex_shader);
	glAttachShader(shader_program_, fragment_shader);
	glBindAttribLocation(shader_program_, kFragmentColour, "fragment_colour");
	glDeleteShader(fragment_shader);
	glLinkProgram(shader_program_);

	GLint link_status = GL_FALSE;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(shader_program_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	/*
		The framework provides a builder class that allows access to all the mesh data	
	*/

	sponza::GeometryBuilder builder;
	const auto& source_meshes = builder.getAllMeshes();

	// We can loop through each mesh in the scene
	for each (const sponza::Mesh& source in source_meshes)
	{
		std::vector<Vertex>vertices;
		Mesh myMesh;
		// Each mesh has an id that you will need to remember for later use
		// obained by calling source.getId()

		myMesh.id = source.getId();

		// To access the actual mesh raw data we can get the array e.g.
		const auto& positions = source.getPositionArray();
		const auto& normals = source.getNormalArray();
		const auto& texcoords = source.getTextureCoordinateArray();
		const auto& elements = source.getElementArray();
		// TODO: you also need to get the normals, elements and texture coordinates in a similar way

		for (int i = 0; i < positions.size(); i++)
		{
			Vertex vertex;

			vertex.position = positions[i];
			vertex.normal = normals[i];
			vertex.texcoord = texcoords[i];
			vertices.push_back(vertex);
		}

		// TODO:
		// Create VBOs for position, normals, elements and texture coordinates

		glGenBuffers(1, &myMesh.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &myMesh.element_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.element_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kNullId);
		myMesh.element_count = elements.size();

		// TODO
		// Create a VAO to wrap all the VBOs

		glGenVertexArrays(1, &myMesh.vao);
		glBindVertexArray(myMesh.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.element_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.vertex_vbo);
		glEnableVertexAttribArray(kVertexPosition);
		glVertexAttribPointer(kVertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(0));
		glEnableVertexAttribArray(kVertexNormal);
		glVertexAttribPointer(kVertexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(kVertexTexcoord);
		glVertexAttribPointer(kVertexTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);
		glBindVertexArray(kNullId);
		
		// TODO: store in a mesh structure and add to a container for later use
		m_meshVector.push_back(myMesh);
	}

}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	glDeleteProgram(shader_program_);
	for (auto& mesh : m_meshVector)
	{
		glDeleteBuffers(1, &mesh.element_vbo);
		glDeleteBuffers(1, &mesh.vertex_vbo);
		glDeleteVertexArrays(1, &mesh.vao);
	}
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear buffers from previous frame
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program_);
	 
	// Compute viewport
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	// Note: the code above is supplied for you and already works

	// TODO: Compute projection matrix
	// You can get the far plane distance, the near plane distance and the fov from
	// scene_->GetCamera().
	//glm::mat4 projection_xform = glm::perspective(glm::radians(fov), aspect_ratio, near_plane_dist, far_plane_dist);
	glm::mat4 projection_xform = glm::perspective(glm::radians(scene_->getCamera().getVerticalFieldOfViewInDegrees()), aspect_ratio, scene_->getCamera().getNearPlaneDistance(), scene_->getCamera().getFarPlaneDistance());
	// TODO: Compute view matrix
	// You can get the camera position, look at and world up from the scene e.g.
	//const auto& camera_pos = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& camera_pos = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& camera_at_pos = (const glm::vec3&)scene_->getCamera().getDirection();
	const auto& world_up = (const glm::vec3&)scene_->getUpDirection();

	// Compute camera view matrix and combine with projection matrix
	//glm::mat4 view_xform = glm::lookAt(camera_pos, camera_at_pos, world_up);
	glm::mat4 view_xform = glm::lookAt(camera_pos, camera_at_pos, world_up);

	// TODO: create combined view * projection matrix and pass to shader as a uniform

	glm::mat4 view_projection_xform = view_xform * projection_xform;

	GLuint view_projection_xform_id = glGetUniformLocation(shader_program_, "view_projection_xform");

	glUniformMatrix4fv(view_projection_xform_id, 1, GL_FALSE, glm::value_ptr(view_projection_xform));

	// TODO: Get light data from scene via scene_->getAllLights()
	// then plug the values into the shader - you may want to leave this until you have a basic scene showing

	// TODO: Render each mesh
	// Loop through your mesh container e.g.
	for (const auto& mesh : m_meshVector)
	{
		// Each mesh can be repeated in the scene so we need to ask the scene for all instances of the mesh
		// and render each instance with its own model matrix
		// To get the instances we need to use the meshId we stored earlier e.g.
		const auto& instances = scene_->getInstancesByMeshId(mesh.id);
		// then loop through all instances
			// for each instance you can call getTransformationMatrix 
			// this then needs passing to the shader as a uniform
		for (const auto& instance : instances)
		{
			sponza::Matrix4x3 mesh_transform = scene_->getInstanceById(instance).getTransformationMatrix();
			//glm::mat4 model_xform{ mesh_transform.m00, mesh_transform.m01, mesh_transform.m02, 0.f, mesh_transform.m10, mesh_transform.m11, mesh_transform.m12, 0.f, mesh_transform.m20, mesh_transform.m21, mesh_transform.m22, 0.f, mesh_transform.m30, mesh_transform.m31, mesh_transform.m32, 1.f };
			glm::mat4 model_xform = glm::translate(glm::mat4(1.f), glm::vec3(mesh_transform.m30, mesh_transform.m31, mesh_transform.m32));

			GLuint model_xform_id = glGetUniformLocation(shader_program_, "model_xform");

			glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.element_count, GL_UNSIGNED_INT, 0);

		}
	
		// Materials - leave this until you get the main scene showing
		// Each instance of the mesh has its own material accessed like so:
		// Get material for this instance
		//const auto& material_id = scene_->getInstanceById(instances.at(i)).getMaterialId();
		//const auto& material = scene_->getMaterialById(material_id);
		// You can then get the material colours from material.XX - you need to pass these to the shader

		// Finally you render the mesh e.g.
		//glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(mesh.vao);
		//glDrawElements(GL_TRIANGLES, mesh.element_count, GL_UNSIGNED_INT, 0);
	}
}
