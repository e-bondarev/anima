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

#include "../animation/animation.h"

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

class Model
{
public:
	Model(const std::string& path);
	~Model();

	OffsetPerNameVec_t bone_map;
	Skeleton_t skeleton;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Assimp::Importer* importer;

private:
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
};