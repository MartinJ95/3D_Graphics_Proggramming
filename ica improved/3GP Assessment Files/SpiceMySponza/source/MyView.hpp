#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class MyView : public tygra::WindowViewDelegate
{
public:
    
    MyView();
    
    ~MyView();
    
	void toggleWireframe() { m_wireframe = !m_wireframe; }

	void toggleTorch() { m_torchActive = !m_torchActive; }

    void setScene(const sponza::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;
    
    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;
    
    void windowViewRender(tygra::Window * window) override;

	//uniform functions
	void setUniformMat4(glm::mat4 matrix, std::string uniform);

	void setUniformVec3(glm::vec3 vector, std::string uniform);

	void setUniformBool(bool boolean, std::string uniform);

	void setUniformFloat(float value, std::string uniform);

	void setUniformSampler(int textureIndex, GLuint Texture, std::string uniform);

private:

	bool m_wireframe = false;

	bool m_torchActive = false;

    const sponza::Context * scene_;

	// Me from here down
	GLuint shader_program_{ 0 };

	const static GLuint kNullId = 0;

	// TODO: define values for your Vertex attributes
	enum vertexAttributes
	{
		kVertexPosition = 0,
		kVertexNormal = 1,
		kVertexTexcoord = 2,
		kVertexTangent = 3
	};

	enum fragmentData
	{
		kFragmentColour = 0
	};

	enum textures
	{
		eDiffuseTexture = 0,
		eSpecularTexture = 1
	};

	struct Texture
	{
		GLuint texture{ 0 };
		std::string name;
	};

	struct Vertex
	{
		sponza::Vector3 position;
		sponza::Vector3 normal;
		sponza::Vector2 texcoord;
		sponza::Vector3 tangent;
	};

	// TODO: create a mesh structure to hold VBO ids etc.
	struct MeshGL
	{
		GLuint vertex_vbo{ 0 };
		
		GLuint elements_vbo{ 0 };

		unsigned int element_count{ 0 };

		GLuint mesh_vao{ 0 };

		unsigned int meshID{ 0 };
	};

	// TODO: create a container of these mesh e.g.
	std::vector<MeshGL> m_meshVector;
	std::vector<Texture> m_textures;
};
