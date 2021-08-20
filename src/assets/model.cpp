#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <spdlog/spdlog.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct VertexBoneData
{
	uint32_t ids[4];
	float weights[4];
};

glm::mat4x4 convert_matrix(aiMatrix4x4 aiMatrix)
{
	glm::mat4x4 result;

	result[0][0] = aiMatrix.a1;
	result[0][1] = aiMatrix.b1;
	result[0][2] = aiMatrix.c1;
	result[0][3] = aiMatrix.d1;
	result[1][0] = aiMatrix.a2;
	result[1][1] = aiMatrix.b2;
	result[1][2] = aiMatrix.c2;
	result[1][3] = aiMatrix.d2;
	result[2][0] = aiMatrix.a3;
	result[2][1] = aiMatrix.b3;
	result[2][2] = aiMatrix.c3;
	result[2][3] = aiMatrix.d3;
	result[3][0] = aiMatrix.a4;
	result[3][1] = aiMatrix.b4;
	result[3][2] = aiMatrix.c4;
	result[3][3] = aiMatrix.d4;

	return result;
}

SkeletalNode::SkeletalNode(aiNode* node, SkeletalNode* parent)
{
	name = std::string(node->mName.data);
	parent = parent;
	transformation = convert_matrix(node->mTransformation);

	for (int i = 0; i < node->mNumChildren; i++)
	{
		children.push_back(new SkeletalNode(node->mChildren[i], this));
	}
}

Model::Model(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene)
	{
		spdlog::error("Failed to load model: {0}", path);
	}

	const aiMesh& mesh = *scene->mMeshes[0];

	std::vector<VertexBoneData> bones_data;

	vertices.resize(mesh.mNumVertices);
	bones_data.resize(mesh.mNumVertices);

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

	indices.reserve(mesh.mNumFaces * 3);

	for (int i = 0; i < mesh.mNumFaces; i++)
	{
		const aiFace* face = &mesh.mFaces[i];
		indices.push_back(face->mIndices[0]);
		indices.push_back(face->mIndices[1]);
		indices.push_back(face->mIndices[2]);
	}

	for (int i = 0; i < mesh.mNumBones; i++)
	{
		uint32_t bone_index = 0;
		std::string bone_name(mesh.mBones[i]->mName.data);

		if (bones_map.find(bone_name) == bones_map.end())
		{
			bone_index = amount_of_bones;
			amount_of_bones++;

			BoneSpace bone_space;

			bone_transforms.push_back(bone_space);
			bone_transforms[bone_index].offset_matrix = convert_matrix(mesh.mBones[i]->mOffsetMatrix);

			bones_map[bone_name] = bone_index;
		}
		else
		{
			bone_index = bones_map[bone_name];
		}

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

	global_inverse_transform = convert_matrix(scene->mRootNode->mTransformation);
	global_inverse_transform = glm::inverse(global_inverse_transform);	

	root_node = new SkeletalNode(scene->mRootNode, nullptr);

	current_transforms.resize(amount_of_bones, glm::mat4(1));
}

static const NodeAnimation* find_node_animation(const Animation& animation, const std::string& node_name)
{
	for (int i = 0; i < animation.channels.size(); i++)
	{
		const NodeAnimation* nodeAnimation = &animation.channels[i];

		if (nodeAnimation->name == node_name)
		{
			return nodeAnimation;
		}
	}

	return nullptr;
}

glm::quat lerp_quat(glm::quat& a, glm::quat& b, float blend)
{
	a = glm::normalize(a);
	b = glm::normalize(b);

	glm::quat result;
	float dotProduct = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	float inversedBlend = 1.0f - blend;

	if (dotProduct < 0.0f)
	{
		result.x = a.x * inversedBlend + blend * -b.x;
		result.y = a.y * inversedBlend + blend * -b.y;
		result.z = a.z * inversedBlend + blend * -b.z;
		result.w = a.w * inversedBlend + blend * -b.w;
	}
	else
	{
		result.x = a.x * inversedBlend + blend * b.x;
		result.y = a.y * inversedBlend + blend * b.y;
		result.z = a.z * inversedBlend + blend * b.z;
		result.w = a.w * inversedBlend + blend * b.w;
	}

	return glm::normalize(result);
}

glm::vec3 lerp_vec3(glm::vec3& first, glm::vec3& second, float blend)
{
	return first + blend * (second - first);
}

template <class T>
static unsigned int get_frame_index(float time, const std::vector<KeyFrame<T>>& keys)
{
	for (int i = 0; i < keys.size() - 1; i++)
	{
		if (time < keys[i + 1].time)
		{
			return i;
		}
	}

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
	float deltaTime = keyFramesPair.next_key_frame.time - keyFramesPair.current_key_frame.time;
	float blend = (time - keyFramesPair.current_key_frame.time) / deltaTime;

	return blend;
}

static void process_node_hierarchy(Model* model, float animationTime, const SkeletalNode* node, const glm::mat4& parentTransform, Animation& animation)
{
	const std::string& nodeName = node->name;
	glm::mat4x4 nodeTransform = node->transformation;

	const NodeAnimation* nodeAnimation = find_node_animation(animation, nodeName);

	if (nodeAnimation)
	{
		KeyFrames<glm::vec3>& scalingKeyFrames = get_key_frames<glm::vec3>(animationTime, nodeAnimation->scale_keys);
		KeyFrames<glm::quat>& rotationKeyFrames = get_key_frames<glm::quat>(animationTime, nodeAnimation->rotation_keys);
		KeyFrames<glm::vec3>& translationKeyFrames = get_key_frames<glm::vec3>(animationTime, nodeAnimation->position_keys);

		glm::vec3 scalingVector = lerp_vec3(
			scalingKeyFrames.current_key_frame.value,
			scalingKeyFrames.next_key_frame.value,
			get_blend_factor<glm::vec3>(scalingKeyFrames, animationTime)
		);

		glm::quat rotationQuat = lerp_quat(
			rotationKeyFrames.current_key_frame.value,
			rotationKeyFrames.next_key_frame.value,
			get_blend_factor<glm::quat>(rotationKeyFrames, animationTime)
		);

		glm::vec3 translationVector = lerp_vec3(
			translationKeyFrames.current_key_frame.value,
			translationKeyFrames.next_key_frame.value,
			get_blend_factor<glm::vec3>(translationKeyFrames, animationTime)
		);

		glm::mat4x4 scalingMatrix = glm::scale(glm::identity<glm::mat4x4>(), scalingVector);
		glm::mat4x4 rotationMatrix = glm::toMat4(rotationQuat);
		glm::mat4x4 translationMatrix = glm::translate(glm::identity<glm::mat4x4>(), translationVector);

		nodeTransform = translationMatrix * rotationMatrix * scalingMatrix;
	}

	glm::mat4x4 globalTransform = parentTransform * nodeTransform;

	if (model->bones_map.find(nodeName) != model->bones_map.end())
	{
		uint32_t boneIndex = model->bones_map[nodeName];
		model->bone_transforms[boneIndex].final_world_matrix = model->global_inverse_transform * globalTransform * model->bone_transforms[boneIndex].offset_matrix;
	}

	for (unsigned int i = 0; i < node->children.size(); i++)
	{
		process_node_hierarchy(model, animationTime, node->children[i], globalTransform, animation);
	}
}

void Model::calculate_pose(float time, Animation& animation)
{
	glm::mat4 root(1);

	float time_in_ticks = time * animation.ticks_per_second;
	float current_time = fmod(time_in_ticks, animation.duration);

	process_node_hierarchy(this, current_time, root_node, root, animation);

	current_transforms.resize(amount_of_bones);

	for (int i = 0; i < amount_of_bones; i++)
	{
		current_transforms[i] = bone_transforms[i].final_world_matrix;
	}
}

Animation::Animation(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene)
	{
		spdlog::error("Failed to load model: {0}", path);
	}

	aiAnimation* assimpAnimation = scene->mAnimations[0];

	name = std::string(assimpAnimation->mName.data);
	duration = static_cast<float>(assimpAnimation->mDuration);

	if (assimpAnimation->mTicksPerSecond != 0.0)
	{
		ticks_per_second = static_cast<float>(assimpAnimation->mTicksPerSecond);
	}
	else
	{
		ticks_per_second = 25.0f;
	}

	for (int i = 0, channelsCount = assimpAnimation->mNumChannels; i < channelsCount; i++)
	{
		aiNodeAnim& assimpNodeAnimation = *assimpAnimation->mChannels[i];
		NodeAnimation nodeAnimation;

		nodeAnimation.name = std::string(assimpNodeAnimation.mNodeName.data);

		for (int j = 0, positionKeysCount = assimpNodeAnimation.mNumPositionKeys; j < positionKeysCount; j++)
		{
			aiVectorKey& assimpPositionKey = assimpNodeAnimation.mPositionKeys[j];
			KeyFrame<glm::vec3> positionKey;

			positionKey.time = static_cast<float>(assimpPositionKey.mTime);
			positionKey.value.x = static_cast<float>(assimpPositionKey.mValue.x);
			positionKey.value.y = static_cast<float>(assimpPositionKey.mValue.y);
			positionKey.value.z = static_cast<float>(assimpPositionKey.mValue.z);

			nodeAnimation.position_keys.push_back(positionKey);
		}

		for (int j = 0, rotationKeysCount = assimpNodeAnimation.mNumRotationKeys; j < rotationKeysCount; j++)
		{
			aiQuatKey& assimpRotationKey = assimpNodeAnimation.mRotationKeys[j];
			KeyFrame<glm::quat> rotationKey;

			rotationKey.time = static_cast<float>(assimpRotationKey.mTime);
			rotationKey.value.x = static_cast<float>(assimpRotationKey.mValue.x);
			rotationKey.value.y = static_cast<float>(assimpRotationKey.mValue.y);
			rotationKey.value.z = static_cast<float>(assimpRotationKey.mValue.z);
			rotationKey.value.w = static_cast<float>(assimpRotationKey.mValue.w);

			nodeAnimation.rotation_keys.push_back(rotationKey);
		}

		for (int j = 0, scaleKeysCount = assimpNodeAnimation.mNumScalingKeys; j < scaleKeysCount; j++)
		{
			aiVectorKey& assimpScaleKey = assimpNodeAnimation.mScalingKeys[j];
			KeyFrame<glm::vec3> scaleKey;

			scaleKey.time = static_cast<float>(assimpScaleKey.mTime);
			scaleKey.value.x = static_cast<float>(assimpScaleKey.mValue.x);
			scaleKey.value.y = static_cast<float>(assimpScaleKey.mValue.y);
			scaleKey.value.z = static_cast<float>(assimpScaleKey.mValue.z);

			nodeAnimation.scale_keys.push_back(scaleKey);
		}

		channels.push_back(nodeAnimation);
	}
}

// Animation* Animation::create_animation(const std::string& path)
// {
// 	Assimp::Importer importer;

// 	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

// 	if (!scene)
// 	{
// 		spdlog::error("Failed to load model: {0}", path);
// 	}

// 	aiAnimation* assimpAnimation = scene->mAnimations[0];

// 	Animation* animation = new Animation();

// 	animation->name = std::string(assimpAnimation->mName.data);
// 	animation->duration = static_cast<float>(assimpAnimation->mDuration);

// 	if (assimpAnimation->mTicksPerSecond != 0.0)
// 	{
// 		animation->ticks_per_second = static_cast<float>(assimpAnimation->mTicksPerSecond);
// 	}
// 	else
// 	{
// 		animation->ticks_per_second = 25.0f;
// 	}

// 	for (int i = 0, channelsCount = assimpAnimation->mNumChannels; i < channelsCount; i++)
// 	{
// 		aiNodeAnim& assimpNodeAnimation = *assimpAnimation->mChannels[i];
// 		NodeAnimation nodeAnimation;

// 		nodeAnimation.name = std::string(assimpNodeAnimation.mNodeName.data);

// 		for (int j = 0, positionKeysCount = assimpNodeAnimation.mNumPositionKeys; j < positionKeysCount; j++)
// 		{
// 			aiVectorKey& assimpPositionKey = assimpNodeAnimation.mPositionKeys[j];
// 			KeyFrame<glm::vec3> positionKey;

// 			positionKey.time = static_cast<float>(assimpPositionKey.mTime);
// 			positionKey.value.x = static_cast<float>(assimpPositionKey.mValue.x);
// 			positionKey.value.y = static_cast<float>(assimpPositionKey.mValue.y);
// 			positionKey.value.z = static_cast<float>(assimpPositionKey.mValue.z);

// 			nodeAnimation.positionKeys.push_back(positionKey);
// 		}

// 		for (int j = 0, rotationKeysCount = assimpNodeAnimation.mNumRotationKeys; j < rotationKeysCount; j++)
// 		{
// 			aiQuatKey& assimpRotationKey = assimpNodeAnimation.mRotationKeys[j];
// 			KeyFrame<glm::quat> rotationKey;

// 			rotationKey.time = static_cast<float>(assimpRotationKey.mTime);
// 			rotationKey.value.x = static_cast<float>(assimpRotationKey.mValue.x);
// 			rotationKey.value.y = static_cast<float>(assimpRotationKey.mValue.y);
// 			rotationKey.value.z = static_cast<float>(assimpRotationKey.mValue.z);
// 			rotationKey.value.w = static_cast<float>(assimpRotationKey.mValue.w);

// 			nodeAnimation.rotationKeys.push_back(rotationKey);
// 		}

// 		for (int j = 0, scaleKeysCount = assimpNodeAnimation.mNumScalingKeys; j < scaleKeysCount; j++)
// 		{
// 			aiVectorKey& assimpScaleKey = assimpNodeAnimation.mScalingKeys[j];
// 			KeyFrame<glm::vec3> scaleKey;

// 			scaleKey.time = static_cast<float>(assimpScaleKey.mTime);
// 			scaleKey.value.x = static_cast<float>(assimpScaleKey.mValue.x);
// 			scaleKey.value.y = static_cast<float>(assimpScaleKey.mValue.y);
// 			scaleKey.value.z = static_cast<float>(assimpScaleKey.mValue.z);

// 			nodeAnimation.scaleKeys.push_back(scaleKey);
// 		}

// 		animation->channels.push_back(nodeAnimation);
// 	}

// 	return animation;
// }