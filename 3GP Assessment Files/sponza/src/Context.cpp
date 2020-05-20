#include <sponza/sponza.hpp>
#include "FirstPersonMovement.hpp"

#include <tcf/tcf.hpp>
#include <tcf/SimpleScene.hpp>

#include <random>
#include <cmath>

using namespace sponza;

/******************************************************************************
*
*
* STOP!
*
* You shouldn't be reading this source file.
* You don't need to know anything about the implementation of the Context.
* Do not base any of your code on how this is implemented,
* to do so would break the concept of encapsulation.
*
* This file contains lots of poor coding practice. It's here just as a proxy
* of a real scene model that would be found in a computer game.
*
*
*****************************************************************************/

Vector3 normalize(const Vector3& v)
{
    float norm = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return Vector3(v.x / norm, v.y / norm, v.z / norm);
}

Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

Context::Context()
{
    start_time_ = std::chrono::system_clock::now();

    if (!readFile("sponza.tcf")) {
        throw std::runtime_error("Failed to read sponza.tcf data file");
    }

    camera_movement_ = std::make_unique<FirstPersonMovement>();
    camera_movement_->init(Vector3(80, 50, 0), 1.5f, 0.5f);

    camera_.setVerticalFieldOfViewInDegrees(75.f);
    camera_.setNearPlaneDistance(1);
    camera_.setFarPlaneDistance(1000);

    update();
}

Context::~Context()
{
}

bool Context::readFile(std::string filepath)
{
    tcf::Reader * reader = tcf::createReader();
    tcf::SimpleScene * tcf_scene = nullptr;

    try {
        reader->openFile(filepath.c_str());
        reader->skipChunk(); // don't care about HEAD
        if (reader->hasChunk()) {
            reader->openChunk();
            if (chunkIsSimpleScene(reader)) {
                tcf_scene = readSimpleScene(reader);
            }
        }
        reader->closeFile();
    } catch (...) {
        if (reader) reader->release();
        if (tcf_scene) tcf_scene->release();
        return false;
    }

    instances_.clear();
    instances_by_mesh_.clear();

    instances_by_mesh_.reserve(tcf_scene->meshCount());
    for (unsigned int i = 0; i < tcf_scene->meshCount(); ++i) {
        const auto * mesh = tcf_scene->findMeshByIndex(i);
        std::vector<InstanceId> instances;
        instances.reserve(mesh->instanceCount());
        instances_.reserve(instances_.size() + mesh->instanceCount());
        for (unsigned int j = 0; j < mesh->instanceCount(); ++j) {
            const auto& model = mesh->transformationArray()[j];
            Instance new_model(100 + (InstanceId)instances_.size());
            new_model.setMeshId(300 + (MeshId)instances_by_mesh_.size());
            new_model.setMaterialId(200);
            new_model.setTransformationMatrix(
                Matrix4x3(model.m00, model.m01, model.m02,
                model.m10, model.m11, model.m12,
                model.m20, model.m21, model.m22,
                model.m30, model.m31, model.m32));
            instances.push_back(new_model.getId());
            instances_.push_back(new_model);
        }
        instances_by_mesh_.push_back(std::move(instances));
    }

    for (auto& instance : instances_)
    {
        instance.setStatic(instance.getMeshId() != 300);
    }

    int redShapes[] = { 35, 36, 37, 38, 39, 40, 41, 42, 69, 70, 71, 72, 73, 74,
        75, 76, 77, 78, 79 };
    int greenShapes[] = { 8, 19, 31, 33, 54, 57, 67, 68, 66, 80 };
    int yellowShapes[] = { 4, 5, 6, 7, 23, 24, 25, 26, 27, 28, 29, 30, 44, 45,
        46, 47, 48, 49, 50, 51, 52, 53 };
    int *shapes[3] = { redShapes, greenShapes, yellowShapes };
    int numberOfShapes[] = {
        sizeof(redShapes) / sizeof(int),
        sizeof(greenShapes) / sizeof(int),
        sizeof(yellowShapes) / sizeof(int) };
    Vector3 diffuse_colours[3] = {
        Vector3(1.f, 0.33f, 0.f),
        Vector3(0.2f, 0.8f, 0.2f),
        Vector3(0.8f, 0.8f, 0.2f)
    };
    Vector3 specular_colours[3] = {
        Vector3(1, 0.2f, 0.2f),
        Vector3(1, 1, 1),
        Vector3(0.8f, 0.8f, 0.2f)
    };
    float shininess[3] = { 0.f, 64.f, 128.f };
	const char* diffuse_textures[3] = {
		"diff1.png",
		"",
		""
	};
	const char* specular_textures[3] = {
		"spec1.png",
        "spec2.png",
        ""
    };
	Material new_material(200);
	new_material.setAmbientColour(Vector3(0.8f, 0.8f, 1));
	new_material.setDiffuseColour(Vector3(0.8f, 0.8f, 0.8f));
	new_material.setDiffuseTexture("diff0.png");
    materials_.push_back(new_material);
    for (int j = 0; j<3; ++j) {
        Material new_material(200 + j + 1);
		new_material.setAmbientColour(Vector3(0.8f, 0.8f, 1));
        new_material.setDiffuseColour(diffuse_colours[j]);
		new_material.setDiffuseTexture(diffuse_textures[j]);
        new_material.setSpecularColour(specular_colours[j]);
        new_material.setShininess(shininess[j]);
		new_material.setSpecularTexture(specular_textures[j]);
		materials_.push_back(new_material);
        for (int i = 0; i<numberOfShapes[j]; ++i) {
            int index = shapes[j][i];
            instances_[index].setMaterialId(new_material.getId());
        }
    }

    reader->release();
    tcf_scene->release();

    return true;
}

void Context::update()
{
    const auto clock_time = std::chrono::system_clock::now() - start_time_;
    const auto clock_millisecs
        = std::chrono::duration_cast<std::chrono::milliseconds>(clock_time);
    const float prev_time = time_seconds_;
    time_seconds_ = 0.001f * clock_millisecs.count();
    const float dt = time_seconds_ - prev_time;

    if (animate_camera_) {
        const float t = -0.3f * time_seconds_;
        const float ct = cosf(t);
        const float rx = ct < 0 ? -120.f : 120.f;
        const float st = sinf(t);
        const float rz = st < 0 ? -20.f : 20.f;
        const float m = 0.1f;
        const float ry = st < 0 ? -30.f : 30.f;
        Vector3 look_at(0, 30, 0);
        Vector3 cam_pos(
            rx * powf(fabs(ct), m),
            50 + ry * powf(fabs(st), m),
            25.f + rz * powf(fabs(st), m));
        camera_.setPosition(cam_pos);
        camera_.setDirection(normalize(look_at - cam_pos));
    } else {
        auto camera_translation_speed = getCamera().getLinearVelocity();
        auto camera_rotation_speed = getCamera().getRotationalVelocity();
        camera_movement_->moveForward(camera_translation_speed.z * dt);
        camera_movement_->moveRight(camera_translation_speed.x * dt);
        camera_movement_->spinHorizontal(camera_rotation_speed.x * dt);
        camera_movement_->spinVertical(camera_rotation_speed.y * dt);
        camera_.setPosition(camera_movement_->position());
        camera_.setDirection(camera_movement_->direction());
    }

    const float t = time_seconds_;
	const bool off_phase = fmodf(time_seconds_, 5) > 3.f;

    const size_t num_of_point_lights = 2;
	const size_t num_of_orb_lights = off_phase ? 10 : 20;
	const size_t num_of_lights = num_of_point_lights + num_of_orb_lights;
    if (lights_.capacity() < num_of_lights)
    {
		lights_.reserve(num_of_lights);
    }

    lights_.clear();
    const LightId base_id = 407;
	for (size_t i = 0; i < num_of_lights; ++i)
	{
		auto light = Light(LightId(base_id + i));
		lights_.push_back(light);
	}

	lights_[0].setPosition(Vector3(75.f, 110.f, -5.f + 15.f * cosf(t)));
    lights_[0].setRange(250.f);

	lights_[1].setPosition(Vector3(-75.f, 110.f, -5.f + 15.f * cosf(1 + t)));
	lights_[1].setRange(250.f);

	auto r = std::default_random_engine(0);
	auto rand = std::uniform_real_distribution<float>(0.6f, 1.f);
	for (size_t i = num_of_point_lights; i < num_of_lights; ++i) {
		auto& light = lights_[i];
		float A = time_seconds_ + i * 6.28f / num_of_orb_lights;
        light.setPosition(Vector3(120.f * cosf(A), 10.f, 40.f * sinf(A)));
		light.setRange(20.f);
		light.setIntensity(Vector3(rand(r), rand(r), rand(r)));
	}

    for (auto& instance : instances_)
    {
        if (instance.getMeshId() != 300) continue;

        auto xform = instance.getTransformationMatrix();
        const float bounce_y = 4;
        xform.m31 = 6.6f + bounce_y * (0.5f + 0.5f * cosf(t));
        instance.setTransformationMatrix(xform);
    }
}

bool Context::toggleCameraAnimation()
{
    return animate_camera_ = !animate_camera_;
}

float Context::getTimeInSeconds() const
{
    return time_seconds_;
}

Vector3 Context::getUpDirection() const
{
    return Vector3(0.f, 1.f, 0.f);
}

Vector3 Context::getAmbientLightIntensity() const
{
    return Vector3(0.2f, 0.2f, 0.f);
}

const Camera& Context::getCamera() const
{
    return camera_;
}

Camera& Context::getCamera()
{
    return camera_;
}

const std::vector<Light>& Context::getAllLights() const
{
    return lights_;
}

const std::vector<Material>& Context::getAllMaterials() const
{
    return materials_;
}

const Material& Context::getMaterialById(MaterialId id) const
{
    return materials_[id - 200];
}

const std::vector<Instance>& Context::getAllInstances() const
{
    return instances_;
}

const Instance& Context::getInstanceById(InstanceId id) const
{
    return instances_[id - 100];
}

const std::vector<InstanceId> Context::getInstancesByMeshId(MeshId id) const
{
    return instances_by_mesh_[id - 300];
}
