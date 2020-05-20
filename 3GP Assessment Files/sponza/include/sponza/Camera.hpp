#pragma once

#include "sponza_fwd.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <memory>

namespace sponza {

class Camera
{
public:
    bool isStatic() const { return false; }

    Vector3 getPosition() const;

    Vector3 getDirection() const;

    float getVerticalFieldOfViewInDegrees() const;

    float getNearPlaneDistance() const;

    float getFarPlaneDistance() const;

    Vector3 getLinearVelocity() const;

    Vector2 getRotationalVelocity() const;

    void setPosition(Vector3 p);
    void setDirection(Vector3 d);
    void setVerticalFieldOfViewInDegrees(float d);
    void setNearPlaneDistance(float n);
    void setFarPlaneDistance(float f);
    void setLinearVelocity(Vector3 v);
    void setRotationalVelocity(Vector2 v);

private:
    Vector3 position{ 0, 0, 0 };
    Vector3 direction{ 0, 0, -1 };
    float vertical_field_of_view_degrees{ 60 };
    float near_plane_distance{ 1 };
    float far_plane_distance{ 10000 };
    Vector3 translation_speed{ 0, 0, 0 };
    Vector2 rotation_speed{ 0, 0 };

};

} // end namespace sponza
