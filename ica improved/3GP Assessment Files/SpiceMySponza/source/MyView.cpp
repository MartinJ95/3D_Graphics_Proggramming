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
	//glBindAttribLocation(shader_program_, kVertexPosition, "vertex_position");
	glBindAttribLocation(shader_program_, kVertexPosition, "vertex_position");
	glBindAttribLocation(shader_program_, kVertexNormal, "vertex_normal");
	glBindAttribLocation(shader_program_, kVertexTexcoord, "vertex_texcoord");
	glBindAttribLocation(shader_program_, kVertexTangent, "vertex_tangent");
	glDeleteShader(vertex_shader);
	glAttachShader(shader_program_, fragment_shader);
	glBindFragDataLocation(shader_program_, kFragmentColour, "fragment_colour");
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
		// Each mesh has an id that you will need to remember for later use
		// obained by calling source.getId()
		MeshGL mesh;

		mesh.meshID = source.getId();
		// To access the actual mesh raw data we can get the array e.g.
		const auto& positions = source.getPositionArray();
		const auto& normals = source.getNormalArray();
		const auto& texcoords = source.getTextureCoordinateArray();
		const auto& tangents = source.getTangentArray();
		
		const auto& elements = source.getElementArray();
		// TODO: you also need to get the normals, elements and texture coordinates in a similar way

		// TODO:
		// Create VBOs for position, normals, elements and texture coordinates

		std::vector<Vertex> vertices;

		for (int i = 0; i < positions.size(); i++)
		{
			Vertex v;
			v.position = positions[i];
			v.normal = normals[i];
			v.texcoord = texcoords[i];
			v.tangent = tangents[i];
			vertices.push_back(v);
		}

		glGenBuffers(1, &mesh.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &mesh.elements_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elements_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kNullId);
		mesh.element_count = elements.size();

		glGenVertexArrays(1, &mesh.mesh_vao);
		glBindVertexArray(mesh.mesh_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elements_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_vbo);
		glEnableVertexAttribArray(kVertexPosition);
		glVertexAttribPointer(kVertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(0));
		glEnableVertexAttribArray(kVertexNormal);
		glVertexAttribPointer(kVertexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(kVertexTexcoord);
		glVertexAttribPointer(kVertexTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(kVertexTangent);
		glVertexAttribPointer(kVertexTangent, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);
		glBindVertexArray(kNullId);

		// TODO
		// Create a VAO to wrap all the VBOs
		
		// TODO: store in a mesh structure and add to a container for later use
		m_meshVector.push_back(mesh);

	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	const auto& sponza_textures = scene_->getAllMaterials();

	std::vector<std::string> texture_names;

	for (auto& t : sponza_textures)
	{
		if (std::find(texture_names.begin(), texture_names.end(), t.getDiffuseTexture()) == texture_names.end())
		{
			if (t.getDiffuseTexture() != "")
			{
				texture_names.push_back(t.getDiffuseTexture());
			}
		}
		if (std::find(texture_names.begin(), texture_names.end(), t.getSpecularTexture()) == texture_names.end())
		{
			if (t.getSpecularTexture() != "")
			{
				texture_names.push_back(t.getSpecularTexture());
			}
		}
	}

	texture_names.push_back("diffNormalMap0");
	texture_names.push_back("diffBumpMap0");

	Texture tex;
	tex.name = "";
	glGenTextures(1, &tex.texture);
	GLubyte data[]{ 255, 255, 255, 255 };
	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, kNullId);

	m_textures.push_back(tex);

	for (auto t : texture_names)
	{
		Texture newTex;
		newTex.name = t;
		tygra::Image texture_image = tygra::createImageFromPngFile("resource:///" + t);
		if (texture_image.doesContainData())
		{
			glGenTextures(1, &newTex.texture);
			glBindTexture(GL_TEXTURE_2D, newTex.texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_image.width(), texture_image.height(), 0, pixel_formats[texture_image.componentsPerPixel()]
				, texture_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, texture_image.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, kNullId);
		}
		m_textures.push_back(newTex);
	}
}

void MyView::setUniformMat4(glm::mat4 matrix, std::string uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader_program_, uniform.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void MyView::setUniformVec3(glm::vec3 vector, std::string uniform)
{
	glUniform3fv(glGetUniformLocation(shader_program_, uniform.c_str()), 1, glm::value_ptr(vector));
}

void MyView::setUniformBool(bool boolean, std::string uniform)
{
	glUniform1i(glGetUniformLocation(shader_program_, uniform.c_str()), boolean);
}

void MyView::setUniformFloat(float value, std::string uniform)
{
	glUniform1f(glGetUniformLocation(shader_program_, uniform.c_str()), value);
}

void MyView::setUniformSampler(int textureIndex, GLuint Texture, std::string uniform)
{
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glUniform1i(glGetUniformLocation(shader_program_, uniform.c_str()), textureIndex);
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
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
	glm::mat4 projection_xform = glm::perspective(glm::radians(scene_->getCamera().getVerticalFieldOfViewInDegrees()),
		aspect_ratio,
		scene_->getCamera().getNearPlaneDistance(),
		scene_->getCamera().getFarPlaneDistance());

	// TODO: Compute view matrix
	// You can get the camera position, look at and world up from the scene e.g.
	//const auto& camera_pos = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& camera_pos = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& camera_dir = (const glm::vec3&)scene_->getCamera().getDirection();
	const auto& world_up = (const glm::vec3&)scene_->getUpDirection();

	// Compute camera view matrix and combine with projection matrix
	//glm::mat4 view_xform = glm::lookAt(camera_pos, camera_at_pos, world_up);

	glm::mat4 view_xform = glm::lookAt(camera_pos, camera_pos + camera_dir, world_up);
	// TODO: create combined view * projection matrix and pass to shader as a uniform

	glm::mat4 view_projection_xform = projection_xform * view_xform;

	// TODO: Get light data from scene via scene_->getAllLights()
	// then plug the values into the shader - you may want to leave this until you have a basic scene showing

	setUniformMat4(view_projection_xform, "view_projection_xform");

	setUniformVec3(camera_pos, "camera_pos");

	setUniformVec3((const glm::vec3&)scene_->getAmbientLightIntensity(), "ambient_lighting");

	const auto& lights = scene_->getAllLights();
	for (int i = 0; i < lights.size(); i++)
	{
		setUniformVec3((const glm::vec3&)lights[i].getPosition(), "lights[" + std::to_string(i) + "].position");
		setUniformVec3((const glm::vec3&)lights[i].getIntensity(), "lights[" + std::to_string(i) + "].intensity");
		setUniformFloat(lights[i].getRange(), "lights[" + std::to_string(i) + "].range");
	}

	setUniformVec3(camera_pos, "torch.position");

	setUniformVec3(camera_dir, "torch.direction");

	setUniformVec3(glm::vec3(1, 1, 1), "torch.intensity");

	setUniformFloat(100.f, "torch.range");

	setUniformFloat(45.f, "torch.field_of_view");

	setUniformBool(m_torchActive, "torch.isOn");

	if (m_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// TODO: Render each mesh
	// Loop through your mesh container e.g.
	for (const auto& mesh : m_meshVector)
	{
		// Each mesh can be repeated in the scene so we need to ask the scene for all instances of the mesh
		// and render each instance with its own model matrix
		// To get the instances we need to use the meshId we stored earlier e.g.
		const auto& instances = scene_->getInstancesByMeshId(mesh.meshID);
		// then loop through all instances
			// for each instance you can call getTransformationMatrix 
			// this then needs passing to the shader as a uniform
	
		for (const auto& instance : instances)
		{

			setUniformMat4((const glm::mat4x3&)scene_->getInstanceById(instance).getTransformationMatrix(), "model_xform");

			const auto& material_id = scene_->getInstanceById(instance).getMaterialId();
			const auto& material = scene_->getMaterialById(material_id);

			setUniformVec3((const glm::vec3&)material.getDiffuseColour(), "material.diffuse_colour");

			setUniformVec3((const glm::vec3&)material.getSpecularColour(), "material.specular_colour");

			setUniformVec3((const glm::vec3&)material.getAmbientColour(), "material.ambient_colour");

			setUniformFloat(material.getShininess(), "material.shininess");

			setUniformBool(material.isShiny(), "material.isShiny");

			for (auto t : m_textures)
			{
				if (material.getDiffuseTexture() == t.name)
				{					
					setUniformSampler(0, t.texture, "diffuseTex");
				}
			}

			for (auto t : m_textures)
			{
				if (material.getSpecularTexture() == t.name)
				{
					setUniformSampler(1, t.texture, "specularTex");
				}
			}

			if (material.getDiffuseTexture() == "diff0")
			{
				for (auto t : m_textures)
				{
					if (t.name == "diffNormalMap0")
					{
						setUniformSampler(3, t.texture, "normalMap");
						setUniformBool(true, "hasNormalMap");
					}
					if (t.name == "diffBumpMap0")
					{
						setUniformSampler(4, t.texture, "bumpMap");
					}
				}
			}
			else
			{
				for (auto t : m_textures)
				{
					if (t.name == "")
					{
						setUniformSampler(3, t.texture, "normalMap");
						setUniformSampler(4, t.texture, "bumpMap");
						setUniformBool(false, "hasNormalMap");
					}
				}
			}
			glBindVertexArray(mesh.mesh_vao);
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
	}
}
