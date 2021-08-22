#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <spdlog/spdlog.h>

static glm::mat4 convert_matrix(aiMatrix4x4 b)
{
	glm::mat4 a;

	a[0][0] = b.a1; a[0][1] = b.b1; a[0][2] = b.c1; a[0][3] = b.d1;
	a[1][0] = b.a2; a[1][1] = b.b2; a[1][2] = b.c2; a[1][3] = b.d2;
	a[2][0] = b.a3; a[2][1] = b.b3; a[2][2] = b.c3; a[2][3] = b.d3;
	a[3][0] = b.a4; a[3][1] = b.b4; a[3][2] = b.c4; a[3][3] = b.d4;

	return a;
}

static void create_skeleton(aiNode* ai_node, Bone& self)
{
	self.name = std::string(ai_node->mName.data);
	self.transformation = convert_matrix(ai_node->mTransformation);

	for (int i = 0; i < ai_node->mNumChildren; i++)
		create_skeleton(ai_node->mChildren[i], self.children.emplace_back());
};

Model::Model(const std::string& path)
{
	importer = new Assimp::Importer();

	const aiScene* scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene)
		spdlog::error("Failed to load model: {0}", path);

	const aiMesh& mesh = *scene->mMeshes[0];

	vertices.resize(mesh.mNumVertices);

	for (int i = 0; i < mesh.mNumVertices; i++)
	{
		const aiVector3D& position = mesh.mVertices[i];
		const aiVector2D& uv = { mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y };
		const aiVector3D& normal = mesh.mNormals[i];

		memcpy(&vertices[i].position, &position, sizeof(glm::vec3));
		memcpy(&vertices[i].uv, &uv, sizeof(glm::vec2));
		memcpy(&vertices[i].normal, &normal, sizeof(glm::vec3));
	}

	indices.resize(mesh.mNumFaces * 3);

	const uint32_t indices_per_face = 3;

	for (int i = 0; i < mesh.mNumFaces; i++)
		for (int j = 0; j < indices_per_face; j++)
			indices[i * indices_per_face + j] = mesh.mFaces[i].mIndices[j];

	struct PerVertexBoneData { uint32_t ids[4]; float weights[4]; };
	std::vector<PerVertexBoneData> bones_data(mesh.mNumVertices);

	std::map<std::string, uint32_t> bones_map;
	uint32_t amount_of_bones = 0;

	for (int i = 0; i < mesh.mNumBones; i++)
	{
		uint32_t bone_index = 0;
		const std::string bone_name(mesh.mBones[i]->mName.data);

		if (bones_map.find(bone_name) == bones_map.end())
			bones_map[bone_name] = bone_index = amount_of_bones++;
		else
			bone_index = bones_map[bone_name];

		for (int j = 0; j < mesh.mBones[i]->mNumWeights; j++)
		{
			const uint32_t vertex_id = mesh.mBones[i]->mWeights[j].mVertexId;
			const float weight = mesh.mBones[i]->mWeights[j].mWeight;

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
		bone_map.push_back({ mesh.mBones[i]->mName.data, convert_matrix(mesh.mBones[i]->mOffsetMatrix) });	

	create_skeleton(scene->mRootNode, skeleton);
}

Model::~Model()
{
	delete importer;
}