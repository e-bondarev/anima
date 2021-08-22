#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "xyapi/gl/vao.h"

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiAnimation;

namespace Assimp
{
	class Importer;
}

struct Vertex
{
    glm::vec3  position;
    glm::vec2  uv;
    glm::vec3  normal;
	glm::ivec4 joint_ids;
	glm::vec4  weights;

    inline static std::vector<VertexBufferLayout> GetLayout() 
	{
        return 
		{
            { 3, sizeof(Vertex), offsetof(Vertex, position) },
            { 2, sizeof(Vertex), offsetof(Vertex, uv) },
            { 3, sizeof(Vertex), offsetof(Vertex, normal) },
            { 4, sizeof(Vertex), offsetof(Vertex, joint_ids) },
            { 4, sizeof(Vertex), offsetof(Vertex, weights) },
        };
    }
};

struct BoneWithName
{
	std::string name;
	glm::mat4 offset;
};

using Bone_t = std::pair<std::string, glm::mat4>;
using BoneMap_t = std::vector<Bone_t>;

struct BoneSpace
{
	glm::mat4 offset_matrix;
	glm::mat4 final_world_matrix;
};

struct Bone
{
	std::string name;
	glm::mat4 transformation;
	std::vector<Bone> children;
};

using Skeleton_t = Bone;

template <class T>
struct KeyFrame
{
	float time;

	T value;
};

template <class T>
struct KeyFrames
{
	KeyFrame<T> current_key_frame;
	KeyFrame<T> next_key_frame;
};

struct BoneAnimation
{
	std::string name;

	std::vector<KeyFrame<glm::vec3>> position_keys;
	std::vector<KeyFrame<glm::quat>> rotation_keys;
	std::vector<KeyFrame<glm::vec3>> scale_keys;
};

class Animation
{
public:
	Animation(const std::string& path);

	std::string name;

	float duration;
	float ticks_per_second;

	std::vector<BoneAnimation> channels;
};

class Avatar
{
public:
	Avatar(std::vector<BoneWithName> bones, Bone skeleton);
	// Avatar(const BoneMap_t& bones, Bone skeleton);

	void calculate_pose(float time, const Animation& animation);
	
	std::vector<BoneSpace> bone_transforms;
	std::vector<glm::mat4> current_transforms;
	glm::mat4 global_inverse_transform;

	Bone skeleton;
	
	std::map<std::string, uint32_t> bones_map;
	uint32_t amount_of_bones{0};

private:
	void process_node_hierarchy(float animation_time, const Animation& animation, Bone& bone, const glm::mat4& parent_transform = glm::mat4(1));

	Avatar(const Avatar&) = delete;
	Avatar& operator=(const Avatar&) = delete;
};

class Model
{
public:
	Model(const std::string& path);
	~Model();

	std::vector<BoneWithName> bones;
	// BoneMap_t bones;
	Skeleton_t skeleton;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Assimp::Importer* importer;

	const aiScene* scene;
	const aiMesh* mesh;

private:
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
};