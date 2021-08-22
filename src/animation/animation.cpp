#include "animation.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <spdlog/spdlog.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

void Avatar::init(const OffsetPerNameVec_t& offset_per_name_vec, const Bone& p_skeleton)
{
	for (int i = 0; i < offset_per_name_vec.size(); i++)
	{
		const std::string& bone_name = offset_per_name_vec[i].first;

		if (bones_map.find(bone_name) == bones_map.end())
		{
			offset_matrices.push_back(offset_per_name_vec[i].second);
			current_transforms.push_back(glm::mat4(1));
			bones_map[bone_name] = i;
		}
	}

	skeleton = p_skeleton;
	global_inverse_transform = glm::inverse(skeleton.transformation);
}

static const BoneAnimation* find_bone_animation(const Animation& animation, const std::string& bone_name)
{
	for (int i = 0; i < animation.channels.size(); i++)
		if (animation.channels[i].name == bone_name)
			return &animation.channels[i];

	return nullptr;
}

template <typename T>
static unsigned int get_frame_index(float time, const std::vector<KeyFrame<T>>& keys)
{
	for (int i = 0; i < keys.size() - 1; i++)
		if (time < keys[i + 1].time)
			return i;

	return 0;
}

template <typename T>
static KeyFrames<T> get_key_frames(float animation_time, const std::vector<KeyFrame<T>>& current)
{
	const uint32_t current_index = get_frame_index<T>(animation_time, current);
	const uint32_t next_index = current_index + 1;

	const KeyFrame<T>& current_key_frame = current[current_index];
	const KeyFrame<T>& next_key_frame = current[next_index];

	return { current_key_frame, next_key_frame };
}

template <typename T>
float KeyFrames<T>::get_blend_factor(float animation_time) const
{
	const float deltaTime = next_key_frame.time - current_key_frame.time;
	const float blend = (animation_time - current_key_frame.time) / deltaTime;

	return blend;
}

void Avatar::process_node_hierarchy(float animation_time, const Animation& animation, Bone& bone, const glm::mat4& parent_transform)
{
	glm::mat4 bone_transform = bone.transformation;

	const BoneAnimation* bone_animation = find_bone_animation(animation, bone.name);

	if (bone_animation)
	{		
		const KeyFrames<glm::vec3>& scaling_key_frames = get_key_frames<glm::vec3>(animation_time, bone_animation->scale_keys);
		const KeyFrames<glm::quat>& rotation_key_frames = get_key_frames<glm::quat>(animation_time, bone_animation->rotation_keys);
		const KeyFrames<glm::vec3>& translation_key_frames = get_key_frames<glm::vec3>(animation_time, bone_animation->position_keys);

		const glm::vec3 scaling_vec = glm::lerp(
			scaling_key_frames.current_key_frame.value,
			scaling_key_frames.next_key_frame.value,
			scaling_key_frames.get_blend_factor(animation_time)
		);

		const glm::quat rotation_quat = glm::lerp(
			rotation_key_frames.current_key_frame.value,
			rotation_key_frames.next_key_frame.value,
			rotation_key_frames.get_blend_factor(animation_time)
		);

		const glm::vec3 translation_vec = glm::lerp(
			translation_key_frames.current_key_frame.value,
			translation_key_frames.next_key_frame.value,
			translation_key_frames.get_blend_factor(animation_time)
		);

		const glm::mat4 scaling = glm::scale(glm::mat4(1), scaling_vec);
		const glm::mat4 rotation = glm::toMat4(rotation_quat);
		const glm::mat4 translation = glm::translate(glm::mat4(1), translation_vec);

		bone_transform = translation * rotation * scaling;
	}

	const glm::mat4 global_transform = parent_transform * bone_transform;

	if (bones_map.find(bone.name) != bones_map.end())
	{
		const uint32_t bone_index = bones_map[bone.name];
		current_transforms[bone_index] = global_inverse_transform * global_transform * offset_matrices[bone_index];
	}

	for (int i = 0; i < bone.children.size(); i++)
		process_node_hierarchy(animation_time, animation, bone.children[i], global_transform);
}

void Avatar::calculate_pose(float time, const Animation& animation)
{
	const float time_in_ticks = time * animation.ticks_per_second;
	const float current_time = fmod(time_in_ticks, animation.duration);

	process_node_hierarchy(current_time, animation, skeleton);
}

Animation::Animation(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene)
		spdlog::error("Failed to load animation: {0}", path);

	const aiAnimation* assimp_animation = scene->mAnimations[0];

	name = std::string(assimp_animation->mName.data);
	duration = static_cast<float>(assimp_animation->mDuration);

	if (assimp_animation->mTicksPerSecond != 0.0)
		ticks_per_second = static_cast<float>(assimp_animation->mTicksPerSecond);
	else
		ticks_per_second = 25.0f;

	for (int i = 0, channels_count = assimp_animation->mNumChannels; i < channels_count; i++)
	{
		const aiNodeAnim& assimp_bone_animation = *assimp_animation->mChannels[i];
		BoneAnimation bone_animation;

		bone_animation.name = std::string(assimp_bone_animation.mNodeName.data);

		for (int j = 0, amount_of_pos_keys = assimp_bone_animation.mNumPositionKeys; j < amount_of_pos_keys; j++)
		{
			const aiVectorKey& assimp_pos_key = assimp_bone_animation.mPositionKeys[j];
			KeyFrame<glm::vec3>& pos_key = bone_animation.position_keys.emplace_back();

			pos_key.time = static_cast<float>(assimp_pos_key.mTime);
			memcpy(&pos_key.value.x, &assimp_pos_key.mValue.x, sizeof(glm::vec3));
		}

		for (int j = 0, amount_of_rot_keys = assimp_bone_animation.mNumRotationKeys; j < amount_of_rot_keys; j++)
		{
			const aiQuatKey& assimp_rot_key = assimp_bone_animation.mRotationKeys[j];
			KeyFrame<glm::quat>& rot_key = bone_animation.rotation_keys.emplace_back();

			rot_key.time = static_cast<float>(assimp_rot_key.mTime);
			memcpy(&rot_key.value[0], &assimp_rot_key.mValue.w, sizeof(float) * 4);
		}

		for (int j = 0, amount_of_scale_keys = assimp_bone_animation.mNumScalingKeys; j < amount_of_scale_keys; j++)
		{
			const aiVectorKey& assimp_scale_key = assimp_bone_animation.mScalingKeys[j];
			KeyFrame<glm::vec3>& scale_key = bone_animation.scale_keys.emplace_back();

			scale_key.time = static_cast<float>(assimp_scale_key.mTime);
			memcpy(&scale_key.value.x, &assimp_scale_key.mValue.x, sizeof(glm::vec3));
		}

		channels.push_back(bone_animation);
	}
}

uint32_t Avatar::get_amount_of_bones() const
{
	return current_transforms.size();
}