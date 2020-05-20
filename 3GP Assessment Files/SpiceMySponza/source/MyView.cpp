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

	std::vector<std::string>texture_files{ "diff0.png", "diff1.png", "spec1.png", "spec2.png" };

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GLuint index = 0;

	for (auto& t : texture_files)
	{
		Texture texture;
		texture.texture_id = t;
		texture.texture_index = index;
		index += 1;
		tygra::Image texture_image = tygra::createImageFromPngFile("resource:///" + t);
		if (texture_image.doesContainData())
		{
			glGenTextures(1, &texture.texture);
			glBindTexture(GL_TEXTURE_2D, texture.texture);
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
		m_textureVector.push_back(texture);
	}
	/*Texture texture;
	texture.texture_id = "";
	glGenTextures(1, &texture.texture);
	GLubyte data[] = { 255, 255, 255, 255 };
	glBindTexture(GL_TEXTURE_2D, texture.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, kNullId);
	texture.texture_index = index;

	m_textureVector.push_back(texture);*/
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
	for (auto& t : m_textureVector)
	{
		glDeleteTextures(1, &t.texture);
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
	glm::mat4 view_xform = glm::lookAt(camera_pos, camera_pos + camera_at_pos, world_up);

	// TODO: create combined view * projection matrix and pass to shader as a uniform

	glm::mat4 view_projection_xform = projection_xform * view_xform;

	GLuint view_projection_xform_id = glGetUniformLocation(shader_program_, "view_projection_xform");

	glUniformMatrix4fv(view_projection_xform_id, 1, GL_FALSE, glm::value_ptr(view_projection_xform));

	// TODO: Get light data from scene via scene_->getAllLights()
	// then plug the values into the shader - you may want to leave this until you have a basic scene showing
	const auto& lights = scene_->getAllLights();
	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 light_position = glm::vec3(lights[i].getPosition().x, lights[i].getPosition().y, lights[i].getPosition().z);
		glm::vec3 light_intensity = glm::vec3(lights[i].getIntensity().x, lights[i].getIntensity().y, lights[i].getIntensity().z);
		float light_range = lights[i].getRange();
		std::string pos = "lights[" + std::to_string(i) + "].position";
		std::string range = "lights[" + std::to_string(i) + "].range";
		std::string intensity = "lights[" + std::to_string(i) + "].intensity";
		GLuint light_position_id = glGetUniformLocation(shader_program_, pos.c_str());
		glUniform3fv(light_position_id, 1, glm::value_ptr(light_position));
		GLuint light_intensity_id = glGetUniformLocation(shader_program_, intensity.c_str());
		glUniform3fv(light_intensity_id, 1, glm::value_ptr(light_intensity));
		GLuint light_range_id = glGetUniformLocation(shader_program_, range.c_str());
		glUniform1f(light_range_id, light_range);
		
	}

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
			glm::mat4 model_xform{ mesh_transform.m00, mesh_transform.m01, mesh_transform.m02, 0.f, mesh_transform.m10, mesh_transform.m11, mesh_transform.m12, 0.f, 
				mesh_transform.m20, mesh_transform.m21, mesh_transform.m22, 0.f, mesh_transform.m30, mesh_transform.m31, mesh_transform.m32, 1.f };
			//glm::mat4 model_xform = glm::translate(glm::mat4(1.f), glm::vec3(mesh_transform.m30, mesh_transform.m31, mesh_transform.m32));

			GLuint model_xform_id = glGetUniformLocation(shader_program_, "model_xform");

			glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

			const auto& material_id = scene_->getInstanceById(instance).getMaterialId();
			const auto& material = scene_->getMaterialById(material_id);

			glm::vec3 material_ambient_colour = glm::vec3(material.getAmbientColour().x, material.getAmbientColour().y, material.getAmbientColour().z);
			glm::vec3 material_diffuse_colour = glm::vec3(material.getDiffuseColour().x, material.getDiffuseColour().y, material.getDiffuseColour().z);
			std::string material_diffuse_texture = material.getDiffuseTexture();
			//int material_id_ = material.getId();
			float material_shininess = material.getShininess();
			glm::vec3 material_specular_colour = glm::vec3(material.getSpecularColour().x, material.getSpecularColour().y, material.getSpecularColour().z);
			std::string material_specualar_texture = material.getSpecularTexture();
			bool material_is_shiny = material.isShiny();

			/*for (auto& t : m_textureVector)
			{
				if (material_diffuse_texture == t.texture_id)
				{
					glActiveTexture(GL_TEXTURE0 + t.texture_index);
					glBindTexture(GL_TEXTURE, t.texture);
					GLuint diffuse_texture_id = glGetUniformLocation(shader_program_, "material.diffuseTex");
					glUniform1i(diffuse_texture_id, t.texture_index);
				}
			}*/

			if (material_diffuse_texture == "")
			{
				bool is_diffuse_tex = false;
				GLuint is_diffuse_tex_id = glGetUniformLocation(shader_program_, "material.diffuseIsTex");
				glUniform1i(is_diffuse_tex_id, is_diffuse_tex);
			}
			else
			{
				for (auto& t : m_textureVector)
				{
					if (material_diffuse_texture == t.texture_id)
					{
						bool is_diffuse_tex = true;
						GLuint is_diffuse_tex_id = glGetUniformLocation(shader_program_, "material.diffuseIsTex");
						glUniform1i(is_diffuse_tex_id, is_diffuse_tex);
						glActiveTexture(GL_TEXTURE0 + t.texture_index);
						glBindTexture(GL_TEXTURE_2D, t.texture);
						GLuint diffuse_texture_id = glGetUniformLocation(shader_program_, "material.diffuseTex");
						glUniform1i(diffuse_texture_id, t.texture_index);
					}
				}
			}

			/*for (auto& t : m_textureVector)
			{
				if (material_specualar_texture == t.texture_id)
				{
					glActiveTexture(GL_TEXTURE0 + t.texture_index);
					glBindTexture(GL_TEXTURE, t.texture);
					GLuint diffuse_texture_id = glGetUniformLocation(shader_program_, "material.specularTex");
					glUniform1i(diffuse_texture_id, t.texture_index);
				}
			}*/

			if (material_specualar_texture == "")
			{
				bool is_specular_tex = false;
				GLuint is_diffuse_tex_id = glGetUniformLocation(shader_program_, "material.specularIsTex");
				glUniform1i(is_diffuse_tex_id, is_specular_tex);
			}
			else
			{
				for (auto& t : m_textureVector)
				{
					if (material_specualar_texture == t.texture_id)
					{
						bool is_specular_tex = true;
						GLuint is_specular_tex_id = glGetUniformLocation(shader_program_, "material.specularIsTex");
						glUniform1i(is_specular_tex_id, is_specular_tex);
						glActiveTexture(GL_TEXTURE0 + t.texture_index);
						glBindTexture(GL_TEXTURE_2D, t.texture);
						GLuint diffuse_texture_id = glGetUniformLocation(shader_program_, "material.specularTex");
						glUniform1i(diffuse_texture_id, t.texture_index);
					}
				}
			}

			/*if (material_specualar_texture == "")
			{
				bool is_specular_tex = false;
				GLuint is_specular_tex_id = glGetUniformLocation(shader_program_, "material.specularIsTex");
				glUniform1i(is_specular_tex_id, is_specular_tex);
			}
			else
			{
				for (auto& t : m_textureVector)
				{
					if (material_specualar_texture == t.texture_id)
					{
						bool is_specular_tex = true;
						GLuint is_specular_tex_id = glGetUniformLocation(shader_program_, "material.specularIsTex");
						glUniform1i(is_specular_tex_id, is_specular_tex);
						glActiveTexture(GL_TEXTURE0 + t.texture_index);
						glBindTexture(GL_TEXTURE_2D, t.texture);
						GLuint diffuse_texture_id = glGetUniformLocation(shader_program_, "material.specularTex");
						glUniform1i(diffuse_texture_id, t.texture_index);
					}
				}
			}*/
			
			glm::vec3 ambient_light = glm::vec3(scene_->getAmbientLightIntensity().x, scene_->getAmbientLightIntensity().y, scene_->getAmbientLightIntensity().z);

			GLuint ambient_light_id = glGetUniformLocation(shader_program_, "ambientLight");

			glUniform3fv(ambient_light_id, 1, glm::value_ptr(ambient_light));

			glm::vec3 camera_position = glm::vec3(scene_->getCamera().getPosition().x, scene_->getCamera().getPosition().y, scene_->getCamera().getPosition().z);

			GLuint camera_position_id = glGetUniformLocation(shader_program_, "cameraPosition");

			glUniform3fv(camera_position_id, 1, glm::value_ptr(camera_position));

			GLuint material_ambient_id = glGetUniformLocation(shader_program_, "material.ambientCol");

			glUniform3fv(material_ambient_id, 1, glm::value_ptr(material_ambient_colour));

			GLuint material_diffuse_colour_id = glGetUniformLocation(shader_program_, "material.diffuseCol");

			glUniform3fv(material_diffuse_colour_id, 1, glm::value_ptr(material_diffuse_colour));

			GLuint material_shininess_id = glGetUniformLocation(shader_program_, "material.shininess");

			glUniform1fv(material_shininess_id, 1, &material_shininess);

			GLuint material_specular_colour_id = glGetUniformLocation(shader_program_, "material.specularCol");

			glUniform3fv(material_specular_colour_id, 1, glm::value_ptr(material_specular_colour));

			GLuint material_is_shiny_id = glGetUniformLocation(shader_program_, "material.isShiny");

			glUniform1i(material_is_shiny_id, material_is_shiny);

			if (m_spotlight)
			{
				glm::vec3 spotLightPosition = (glm::vec3&)scene_->getCamera().getPosition();
				GLuint spotlight_position_id = glGetUniformLocation(shader_program_, "spotlight.position");
				glUniform3fv(spotlight_position_id, 1, glm::value_ptr(spotLightPosition));

				glm::vec3 spotLightDirection = (glm::vec3&)scene_->getCamera().getDirection();
				GLuint spotlight_direction_id = glGetUniformLocation(shader_program_, "spotlight.direction");
				glUniform3fv(spotlight_direction_id, 1, glm::value_ptr(spotLightDirection));

				glm::vec3 spotLightIntensity = glm::vec3(1, 1, 1);
				GLuint spotlight_intensity_id = glGetUniformLocation(shader_program_, "spotlight.intensity");
				glUniform3fv(spotlight_intensity_id, 1, glm::value_ptr(spotLightIntensity));

				float spotLightFieldOfView = 45.f;
				GLuint spotlight_fov_id = glGetUniformLocation(shader_program_, "spotlight.field_of_view");
				glUniform1f(spotlight_fov_id, spotLightFieldOfView);

				float spotLightRange = 100.f;
				GLuint spotlight_range_id = glGetUniformLocation(shader_program_, "spotlight.range");
				glUniform1f(spotlight_range_id, spotLightRange);

				bool spotLightActive = true;
				GLuint spotlight_active_id = glGetUniformLocation(shader_program_, "spotLightActive");
				glUniform1i(spotlight_active_id, spotLightActive);
			}
			else
			{
				bool spotLightActive = false;
				GLuint spotlight_active_id = glGetUniformLocation(shader_program_, "spotLightActive");
				glUniform1i(spotlight_active_id, spotLightActive);
			}
			
			if (m_wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}


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
