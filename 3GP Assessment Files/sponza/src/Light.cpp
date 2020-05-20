#include <sponza/sponza.hpp>

using namespace sponza;

Light::Light(LightId i) : id(i)
{
}

LightId Light::getId() const
{
    return id;
}

bool Light::isStatic() const
{
    return is_static;
}

void Light::setStatic(bool b)
{
    is_static = b;
}

Vector3 Light::getPosition() const
{
    return position;
}

void Light::setPosition(Vector3 p)
{
    position = p;
}

float Light::getRange() const
{
    return range;
}

void Light::setRange(float r)
{
    range = r;
}

Vector3 Light::getIntensity() const
{
    return intensity;
}

void Light::setIntensity(Vector3 i)
{
    intensity = i;
}
