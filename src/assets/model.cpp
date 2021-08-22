#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <spdlog/spdlog.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

static glm::mat4x4 convert_matrix(aiMatrix4x4 b)
{
	glm::mat4x4 a;

	a[0][0] = b.a1; a[0][1] = b.b1; a[0][2] = b.c1; a[0][3] = b.d1;
	a[1][0] = b.a2; a[1][1] = b.b2; a[1][2] = b.c2; a[1][3] = b.d2;
	a[2][0] = b.a3; a[2][1] = b.b3; a[2][2] = b.c3; a[2][3] = b.d3;
	a[3][0] = b.a4; a[3][1] = b.b4; a[3][2] = b.c4; a[3][3] = b.d4;

	return a;
}

// void Bone::Init(aiNode* ai_node)
// {
// 	name = std::string(ai_node->mName.data);
// 	transformation = convert_matrix(ai_node->mTransformation);

// 	for (int i = 0; i < ai_node->mNumChildren; i++)
// 		children.emplace_back().Init(ai_node->mChildren[i]);
// }

static void create_skeleton(aiNode* ai_node, Bone& self)
{
	self.name = std::string(ai_node->mName.data);
	self.transformation = convert_matrix(ai_node->mTransformation);

	for (int i = 0; i < ai_node->mNumChildren; i++)
	{
		create_skeleton(ai_node->mChildren[i], self.children.emplace_back());
	}
};

Model::Model(const std::string& path)
{
	importer = new Assimp::Importer();

	const aiScene* scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene)
		spdlog::error("Failed to load model: {0}", path);

	const aiMesh& mesh = *scene->mMeshes[0];

	this->scene = scene;
	this->mesh = scene->mMeshes[0];

	vertices.resize(mesh.mNumVertices);

	for (int i = 0; i < mesh.mNumVertices; i++)
	{
		const aiVector3D& position = mesh.mVertices[i];
		const aiVector2D& uv = { mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y };
		const aiVector3D& normal = mesh.mNormals[i];

		Vertex& vertex = vertices[i];

		memcpy(&vertex.position, &position, sizeof(glm::vec3));
		memcpy(&vertex.uv, &uv, sizeof(glm::vec2));
		memcpy(&vertex.normal, &normal, sizeof(glm::vec3));
	}

	indices.resize(mesh.mNumFaces * 3);

	const uint32_t indices_per_face = 3;

	for (int i = 0; i < mesh.mNumFaces; i++)
		for (int j = 0; j < indices_per_face; j++)
			indices[i * indices_per_face + j] = mesh.mFaces[i].mIndices[j];

	struct PerVertexBoneData { uint32_t ids[4]; float weights[4]; };
	std::vector<PerVertexBoneData> bones_data(mesh.mNumVertices);

	std::map<std::string, uint32_t> bones_map;
	uint32_t amount_of_bones{0};

	for (int i = 0; i < mesh.mNumBones; i++)
	{
		uint32_t bone_index = 0;
		std::string bone_name(mesh.mBones[i]->mName.data);

		if (bones_map.find(bone_name) == bones_map.end())
			bones_map[bone_name] = bone_index = amount_of_bones++;
		else
			bone_index = bones_map[bone_name];

		for (int j = 0; j < mesh.mBones[i]->mNumWeights; j++)
		{
			uint32_t vertex_id = mesh.mBones[i]->mWeights[j].mVertexId;
			float weight = mesh.mBones[i]->mWeights[j].mWeight;

			for (int k = 0; k < 4; k++)
			{
				if (bones_data[vertex_id].weights[k] == 0.0)
				{
					bones_data[vertex_id].ids[k] = bone_index;
					bones_data[vertex_id].weights[k] = weight;
					break;
				}
			}
		}
	}

	for (int i = 0; i < mesh.mNumVertices; i++)
	{
		memcpy(&vertices[i].joint_ids[0], &bones_data[i].ids[0], sizeof(glm::vec4));
		memcpy(&vertices[i].weights[0], &bones_data[i].weights[0], sizeof(glm::vec4));
	}

	for (int i = 0; i < mesh.mNumBones; i++)
		bones.push_back({ mesh.mBones[i]->mName.data, convert_matrix(mesh.mBones[i]->mOffsetMatrix) });	
		
	// for (int i = 0; i < mesh.mNumBones; i++)
	// 	bones[std::string(mesh.mBones[i]->mName.data)] = convert_matrix(mesh.mBones[i]->mOffsetMatrix);

	create_skeleton(scene->mRootNode, skeleton);
}

Model::~Model()
{
	delete importer;
}

Avatar::Avatar(std::vector<BoneWithName> BONES, Bone p_skeleton) : skeleton {p_skeleton}
{
	for (int i = 0; i < BONES.size(); i++)
	{
		if (bones_map.find(BONES[i].name) == bones_map.end())
		{
			bone_transforms.emplace_back().offset_matrix = BONES[i].offset;
			bones_map[BONES[i].name] = i;
			spdlog::info("{0}. {1}", i, BONES[i].name);
		}
	}

	// int i = 0;
	// for (auto& BONE : BONES)
	// {
	// 	if (bones_map.find(BONE.first) == bones_map.end())
	// 	{
	// 		bone_transforms.emplace_back().offset_matrix = BONE.second;
	// 		bones_map[BONE.first] = i;
	// 		spdlog::info("{0}. {1}", i, BONE.first);
	// 	}
	// 	i++;
	// }

	amount_of_bones = BONES.size();

	global_inverse_transform = glm::inverse(skeleton.transformation);

	current_transforms.resize(amount_of_bones, glm::mat4(1));
}

static const BoneAnimation* find_bone_animation(const Animation& animation, const std::string& bone_name)
{
	for (int i = 0; i < animation.channels.size(); i++)
		if (animation.channels[i].name == bone_name)
			return &animation.channels[i];

	return nullptr;
}

static glm::quat lerp_quat(const glm::quat& a, const glm::quat& b, const float blend)
{
	struct {
		glm::quat a;
		glm::quat b;
	} normalized;

	normalized.a = glm::normalize(a);
	normalized.b = glm::normalize(b);

	glm::quat result;
	const float dot_product = normalized.a.x * normalized.b.x + normalized.a.y * normalized.b.y + normalized.a.z * normalized.b.z + normalized.a.w * normalized.b.w;
	const float inversed_blend = 1.0f - blend;

	if (dot_product < 0.0f)
	{
		result.x = normalized.a.x * inversed_blend + blend * -normalized.b.x;
		result.y = normalized.a.y * inversed_blend + blend * -normalized.b.y;
		result.z = normalized.a.z * inversed_blend + blend * -normalized.b.z;
		result.w = normalized.a.w * inversed_blend + blend * -normalized.b.w;
	}
	else
	{
		result.x = normalized.a.x * inversed_blend + blend * normalized.b.x;
		result.y = normalized.a.y * inversed_blend + blend * normalized.b.y;
		result.z = normalized.a.z * inversed_blend + blend * normalized.b.z;
		result.w = normalized.a.w * inversed_blend + blend * normalized.b.w;
	}

	return result;
}

static glm::vec3 lerp_vec3(const glm::vec3& first, const glm::vec3& second, const float blend)
{
	return first + blend * (second - first);
}

template <class T>
static unsigned int get_frame_index(float time, const std::vector<KeyFrame<T>>& keys)
{
	for (int i = 0; i < keys.size() - 1; i++)
		if (time < keys[i + 1].time)
			return i;

	return 0;
}

template <class T>
static KeyFrames<T> get_key_frames(float animation_time, const std::vector<KeyFrame<T>>& current)
{
	const uint32_t current_index = get_frame_index<T>(animation_time, current);
	const uint32_t next_index = current_index + 1;

	const KeyFrame<T>& current_key_frame = current[current_index];
	const KeyFrame<T>& next_key_frame = current[next_index];

	return { current_key_frame, next_key_frame };
}

template <class T>
static float get_blend_factor(const KeyFrames<T>& keyFramesPair, float time)
{
	const float deltaTime = keyFramesPair.next_key_frame.time - keyFramesPair.current_key_frame.time;
	const float blend = (time - keyFramesPair.current_key_frame.time) / deltaTime;

	return blend;
}

void Avatar::process_node_hierarchy(float animation_time, const Animation& animation, Bone& bone, const glm::mat4& parent_transform)
{
	glm::mat4 bone_transform = bone.transformation;

	const BoneAnimation* bone_animation = find_bone_animation(animation, bone.name);

	if (bone_animation)
	{		
		KeyFrames<glm::vec3>& scaling_key_frames = get_key_frames<glm::vec3>(animation_time, bone_animation->scale_keys);
		KeyFrames<glm::quat>& rotation_key_frames = get_key_frames<glm::quat>(animation_time, bone_animation->rotation_keys);
		KeyFrames<glm::vec3>& translation_key_frames = get_key_frames<glm::vec3>(animation_time, bone_animation->position_keys);

		const glm::vec3 scaling_vec = lerp_vec3(
			scaling_key_frames.current_key_frame.value,
			scaling_key_frames.next_key_frame.value,
			get_blend_factor<glm::vec3>(scaling_key_frames, animation_time)
		);

		const glm::quat rotation_quat = lerp_quat(
			rotation_key_frames.current_key_frame.value,
			rotation_key_frames.next_key_frame.value,
			get_blend_factor<glm::quat>(rotation_key_frames, animation_time)
		);

		const glm::vec3 translation_vec = lerp_vec3(
			translation_key_frames.current_key_frame.value,
			translation_key_frames.next_key_frame.value,
			get_blend_factor<glm::vec3>(translation_key_frames, animation_time)
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
		bone_transforms[bone_index].final_world_matrix = global_inverse_transform * global_transform * bone_transforms[bone_index].offset_matrix;
	}

	for (int i = 0; i < bone.children.size(); i++)
		process_node_hierarchy(animation_time, animation, bone.children[i], global_transform);
}

void Avatar::calculate_pose(float time, const Animation& animation)
{
	const float time_in_ticks = time * animation.ticks_per_second;
	const float current_time = fmod(time_in_ticks, animation.duration);

	process_node_hierarchy(current_time, animation, skeleton);

	current_transforms.resize(amount_of_bones);

	for (int i = 0; i < amount_of_bones; i++)
		current_transforms[i] = bone_transforms[i].final_world_matrix;
}

Animation::Animation(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene)
		spdlog::error("Failed to load model: {0}", path);

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
			aiVectorKey& assimp_pos_key = assimp_bone_animation.mPositionKeys[j];
			KeyFrame<glm::vec3>& pos_key = bone_animation.position_keys.emplace_back();

			pos_key.time = static_cast<float>(assimp_pos_key.mTime);
			memcpy(&pos_key.value.x, &assimp_pos_key.mValue.x, sizeof(glm::vec3));
		}

		for (int j = 0, amount_of_rot_keys = assimp_bone_animation.mNumRotationKeys; j < amount_of_rot_keys; j++)
		{
			aiQuatKey& assimp_rot_key = assimp_bone_animation.mRotationKeys[j];
			KeyFrame<glm::quat>& rot_key = bone_animation.rotation_keys.emplace_back();

			rot_key.time = static_cast<float>(assimp_rot_key.mTime);
			memcpy(&rot_key.value[0], &assimp_rot_key.mValue.w, sizeof(float) * 4);
		}

		for (int j = 0, amount_of_scale_keys = assimp_bone_animation.mNumScalingKeys; j < amount_of_scale_keys; j++)
		{
			aiVectorKey& assimp_scale_key = assimp_bone_animation.mScalingKeys[j];
			KeyFrame<glm::vec3>& scale_key = bone_animation.scale_keys.emplace_back();

			scale_key.time = static_cast<float>(assimp_scale_key.mTime);
			memcpy(&scale_key.value.x, &assimp_scale_key.mValue.x, sizeof(glm::vec3));
		}

		channels.push_back(bone_animation);
	}
}