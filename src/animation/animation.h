#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>

struct Bone
{
	std::string name;
	glm::mat4 transformation;
	std::vector<Bone> children;
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

using Skeleton_t = Bone;
using OffsetPerName_t = std::pair<std::string, glm::mat4>;
using OffsetPerNameVec_t = std::vector<OffsetPerName_t>;

class Avatar
{
public:
	Avatar() = default;

	void init(const OffsetPerNameVec_t& bones, const Bone& skeleton);
	void calculate_pose(float time, const Animation& animation);
	
	std::vector<glm::mat4> current_transforms;
	std::vector<glm::mat4> offset_matrices;
	glm::mat4 global_inverse_transform;

	Bone skeleton;

	std::map<std::string, uint32_t> bones_map;

	uint32_t get_amount_of_bones() const;

private:
	void process_node_hierarchy(float animation_time, const Animation& animation, Bone& bone, const glm::mat4& parent_transform = glm::mat4(1));

	Avatar(const Avatar&) = delete;
	Avatar& operator=(const Avatar&) = delete;
};