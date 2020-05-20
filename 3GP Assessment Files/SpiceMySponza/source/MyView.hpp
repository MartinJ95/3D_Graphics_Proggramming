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
    
    void setScene(const sponza::Context * scene);

	void toggleWireframe() { m_wireframe = !m_wireframe; }

	void toggleSpotLight() { m_spotlight = !m_spotlight; }

private:

    void windowViewWillStart(tygra::Window * window) override;
    
    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;
    
    void windowViewRender(tygra::Window * window) override;

private:

	bool m_wireframe = false;

	bool m_spotlight = false;

    const sponza::Context * scene_;

	// Me from here down
	GLuint shader_program_{ 0 };

	const static GLuint kNullId = 0;

	// TODO: define values for your Vertex attributes
	enum VertexAttribIndexes
	{
		kVertexPosition = 0,
		kVertexNormal = 1,
		kVertexTexcoord = 2
	};

	enum FragmentDataIndexes
	{
		kFragmentColour = 0
	};

	struct Vertex
	{
		sponza::Vector3 position;
		sponza::Vector3 normal;
		sponza::Vector2 texcoord;
	};

	struct Texture
	{
		std::string texture_id{ "" };
		GLuint texture_index{ 0 };
		GLuint texture{ 0 };
	};

	// TODO: create a mesh structure to hold VBO ids etc.

	struct Mesh
	{
		GLuint id{ 0 };

		GLuint vertex_vbo{ 0 };

		GLuint element_vbo{ 0 };

		GLuint vao{ 0 };

		int element_count{ 0 };
	};

	// TODO: create a container of these mesh e.g.
	std::vector<Mesh> m_meshVector;
	std::vector<Texture> m_textureVector;
};
