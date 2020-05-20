#pragma once

#include "sponza_fwd.hpp"
#include <string>

namespace sponza {

class Material
{
public:
    Material(MaterialId i);

    MaterialId getId() const;

    bool isStatic() const { return true; }

    Vector3 getAmbientColour() const;
	void setAmbientColour(Vector3 c);

    Vector3 getDiffuseColour() const;
    void setDiffuseColour(Vector3 c);

	const std::string& getDiffuseTexture() const;
	void setDiffuseTexture(const std::string& s);

    Vector3 getSpecularColour() const;
    void setSpecularColour(Vector3 c);

    float getShininess() const;
    void setShininess(float s);

	const std::string& getSpecularTexture() const;
	void setSpecularTexture(const std::string& s);

    bool isShiny() const;


private:
    MaterialId id{ 0 };
    Vector3 ambient_colour{ 0.5f, 0.5f, 0.5f };
    Vector3 diffuse_colour{ 0.5f, 0.5f, 0.5f };
    std::string diffuse_texture;
    Vector3 specular_colour{ 1.f, 1.f, 1.f };
    float shininess{ 0.f };
	std::string specular_texture;

};

} // end namespace sponza
