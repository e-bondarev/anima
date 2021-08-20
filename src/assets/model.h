#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "xyapi/gl/vao.h"

struct aiNode;
struct aiAnimation;

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

struct BoneSpace
{
	glm::mat4 offset_matrix;
	glm::mat4 final_world_matrix;
};

struct SkeletalNode
{
	std::string name;

	glm::mat4 transformation;

	SkeletalNode* parent;
	std::vector<std::shared_ptr<SkeletalNode>> children;

	SkeletalNode(aiNode* node, SkeletalNode* parent);
};

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

struct NodeAnimation
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

	std::vector<NodeAnimation> channels;
};

class Avatar
{
public:
	Avatar() = default;

	void calculate_pose(float time, Animation& animation);
	
	std::vector<BoneSpace> bone_transforms;
	std::vector<glm::mat4> current_transforms;
	glm::mat4 global_inverse_transform;
	std::unique_ptr<SkeletalNode> root_node;
	std::map<std::string, uint32_t> bones_map;
	uint32_t amount_of_bones{0};

private:
	Avatar(const Avatar&) = delete;
	Avatar& operator=(const Avatar&) = delete;
};

class Model
{
public:
	Model(const std::string& path);
	
	Avatar* avatar;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};