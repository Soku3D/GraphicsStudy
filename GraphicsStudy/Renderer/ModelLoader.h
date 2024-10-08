#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "MeshData.h"
#include "Vertex.h"
#include "AnimationClip.h"
#include "directxtk\SimpleMath.h"
namespace Renderer {

	template<typename Vertex, typename Index>
	class ModelLoader {
	public:
		ModelLoader() {
			basePath = std::filesystem::current_path().string();
			basePath += "/Models/";
		};
		~ModelLoader() {};

	public:
		int meshCount = 0;
		std::string basePath;
		std::vector<MeshData<Vertex, Index>> meshes;
		Animation::AnimationData m_animeData;

	public:
		void FindDeformingBones(const aiScene* scene);
		void UpdateBoneTree(const aiNode* node, int* count);
		const aiNode* FindParentsBoneId(const aiNode* node);
		void ExtractBonePositions(aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene);

		template<typename Vertex, typename Index>
		void Load(std::string filename, bool loadAnimation = false, DirectX::SimpleMath::Matrix tr = DirectX::SimpleMath::Matrix())
		{
			if (loadAnimation)
			{
				basePath += "Animations/";
			}

			Assimp::Importer importer;

			const aiScene* pScene = importer.ReadFile(
				this->basePath + filename,
				aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

			if (pScene == nullptr)
			{
				std::cout << "Can't Load Model " + basePath + filename << std::endl;
				return;
			}

			FindDeformingBones(pScene);
			int count = 0;
			UpdateBoneTree(pScene->mRootNode, &count);

			m_animeData.bonePositions.resize(m_animeData.boneNameToId.size());
			m_animeData.boneIdToName.resize(m_animeData.boneNameToId.size());
			for (auto& i : m_animeData.boneNameToId)
				m_animeData.boneIdToName[i.second] = i.first;

			aiMatrix4x4 mat = aiMatrix4x4();

			ExtractBonePositions(pScene->mRootNode, mat, pScene);



			m_animeData.boneParentsId.resize(m_animeData.boneNameToId.size(), -1);
			m_animeData.tPoseTransforms.resize(m_animeData.boneNameToId.size());
			ProcessNode<Vertex, Index>(pScene->mRootNode, pScene, tr, loadAnimation);


			if (loadAnimation)
			{
				ReadAnimation(pScene);
			}
		}
		void ReadAnimation(const aiScene* pScene)
		{
			using DirectX::SimpleMath::Quaternion;

			m_animeData.clips.resize(pScene->mNumAnimations);

			for (uint32_t i = 0; i < pScene->mNumAnimations; i++) {

				auto& clip = m_animeData.clips[i];

				const aiAnimation* ani = pScene->mAnimations[i];

				clip.duration = ani->mDuration;
				clip.ticksPerSec = ani->mTicksPerSecond;
				clip.keys.resize(m_animeData.boneNameToId.size());
				clip.numChannels = ani->mNumChannels;

				// i번쨰 노드(채널 / 메쉬)
				for (uint32_t c = 0; c < ani->mNumChannels; c++) {
					const aiNodeAnim* nodeAnim = ani->mChannels[c];
					const int boneId =
						m_animeData.boneNameToId[nodeAnim->mNodeName.C_Str()];
					if (boneId == 7) {
						int parentsId = m_animeData.boneParentsId[boneId];
						std::cout << m_animeData.boneIdToName[parentsId];
					}
					clip.keys[boneId].resize(nodeAnim->mNumPositionKeys);
					for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++) {
						const auto pos = nodeAnim->mPositionKeys[k].mValue;
						const auto rot = nodeAnim->mRotationKeys[k].mValue;
						const auto scale = nodeAnim->mScalingKeys[k].mValue;
						auto& key = clip.keys[boneId][k];
						key.pos = { pos.x, pos.y, pos.z };
						key.rot = Quaternion(rot.x, rot.y, rot.z, rot.w);
						key.scale = { scale.x, scale.y, scale.z };
					}
				}
			}
		}

		template<typename Vertex, typename Index>
		void ProcessNode(aiNode* node, const aiScene* scene,
			DirectX::SimpleMath::Matrix tr, bool loadAnimation)
		{
			using DirectX::SimpleMath::Matrix;
			Matrix m(&node->mTransformation.a1);
			m = m.Transpose() * tr;

			if (node->mParent && m_animeData.boneNameToId.count(node->mName.C_Str()) &&
				FindParentsBoneId(node->mParent) && loadAnimation) {
				const auto boneId = m_animeData.boneNameToId[node->mName.C_Str()];
				m_animeData.boneParentsId[boneId] =
					m_animeData.boneNameToId[FindParentsBoneId(node->mParent)->mName.C_Str()];
				m_animeData.tPoseTransforms[boneId] = m;
			}
			
			for (UINT i = 0; i < node->mNumMeshes; i++) {

				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				auto newMesh = this->ProcessMesh<Vertex, Index>(mesh, scene, loadAnimation);
				newMesh.m_name = mesh->mName.C_Str();

				for (auto& v : newMesh.m_vertices) {
					v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
				}

				meshes.push_back(newMesh);
			}

			for (UINT i = 0; i < node->mNumChildren; i++) {
				this->ProcessNode<Vertex, Index>(node->mChildren[i], scene, m, loadAnimation);
			}
		}

		template<typename Vertex, typename Index>
		MeshData<Vertex, Index> ProcessMesh(aiMesh* mesh, const aiScene* scene, bool loadAnimation) {
			// Data to fill
			std::vector<Vertex> vertices;
			std::vector<Index> indices;

			// Walk through each of the mesh's vertices
			for (UINT i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;

				vertex.position.x = mesh->mVertices[i].x;
				vertex.position.y = mesh->mVertices[i].y;
				vertex.position.z = mesh->mVertices[i].z;

				if (mesh->mNormals != nullptr) {
					vertex.normal.x = -mesh->mNormals[i].x;
					vertex.normal.y = mesh->mNormals[i].y;
					vertex.normal.z = -mesh->mNormals[i].z;
					DirectX::SimpleMath::Vector3 n = vertex.normal;
					n.Normalize();
					vertex.normal = n;
				}
				else {
					std::cout << "NULL Normals" << std::endl;
				}


				if (mesh->mTextureCoords[0]) {
					vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
					vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
				}

				vertices.push_back(vertex);
			}

			for (UINT i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (UINT j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			if (loadAnimation && mesh->HasBones()) {

				std::vector<std::vector<float>> boneWeights(vertices.size());
				std::vector<std::vector<uint8_t>> boneIndices(vertices.size());

				m_animeData.offsetMatrices.resize(m_animeData.boneNameToId.size());
				m_animeData.boneTransforms.resize(m_animeData.boneNameToId.size());

				int count = 0;
				for (uint32_t i = 0; i < mesh->mNumBones; i++) {
					const aiBone* bone = mesh->mBones[i];
					const uint32_t boneId = m_animeData.boneNameToId[bone->mName.C_Str()];

					m_animeData.offsetMatrices[boneId] =
						DirectX::SimpleMath::Matrix((float*)&bone->mOffsetMatrix).Transpose();

					for (uint32_t j = 0; j < bone->mNumWeights; j++) {
						aiVertexWeight weight = bone->mWeights[j];
						assert(weight.mVertexId < boneIndices.size());
						boneIndices[weight.mVertexId].push_back(boneId);
						boneWeights[weight.mVertexId].push_back(weight.mWeight);
					}
				}

				for (int i = 0; i < vertices.size(); i++) {

					for (int j = 0; j < boneWeights[i].size(); j++) {
						vertices[i].blendWeights[j] = boneWeights[i][j];
						vertices[i].boneIndices[j] = boneIndices[i][j];
					}
				}
			}
			MeshData<Vertex, Index> newMesh;
			newMesh.m_vertices = vertices;
			newMesh.m_indices = indices;



			if (mesh->mMaterialIndex >= 0) {
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
					aiString filepath;
					material->GetTexture(aiTextureType_DIFFUSE, 0, &filepath);

					std::wstring fullPath =
						std::wstring(std::filesystem::path(filepath.C_Str())
							.filename()
							.wstring());

					newMesh.m_texturePath = fullPath;
					std::wcout << fullPath << std::endl;
				}
			}
			return newMesh;
		}
	};
	template<typename Vertex, typename Index>
	inline void ModelLoader<Vertex, Index>::FindDeformingBones(const aiScene* scene)
	{
		for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
			const auto* mesh = scene->mMeshes[i];
			if (mesh->HasBones()) {
				for (uint32_t i = 0; i < mesh->mNumBones; i++) {
					const aiBone* bone = mesh->mBones[i];

					m_animeData.boneNameToId[bone->mName.C_Str()] = -1;
				}
			}
		}
	}
	template<typename Vertex, typename Index>
	inline void ModelLoader<Vertex, Index>::UpdateBoneTree(const aiNode* node, int* count)
	{
		if (node) {
			if (m_animeData.boneNameToId.count(node->mName.C_Str()))
			{
				m_animeData.boneNameToId[node->mName.C_Str()] = *count;
				*count += 1;


			}
			for (size_t i = 0; i < node->mNumChildren; i++)
			{
				UpdateBoneTree(node->mChildren[i], count);
			}
		}
	}
	template<typename Vertex, typename Index>
	inline const aiNode* ModelLoader<Vertex, Index>::FindParentsBoneId(const aiNode* node)
	{
		if (!node)
			return nullptr;
		if (m_animeData.boneNameToId.count(node->mName.C_Str()) > 0)
			return node;
		return FindParentsBoneId(node->mParent);
	}
	template<typename Vertex, typename Index>
	inline void ModelLoader<Vertex, Index>::ExtractBonePositions(aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene)
	{
		aiMatrix4x4 nodeTransform = parentTransform * node->mTransformation;

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[i];
			for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
				aiBone* bone = mesh->mBones[j];
				if (bone->mName == node->mName) {
					aiMatrix4x4 boneOffset = bone->mOffsetMatrix;
					aiMatrix4x4 globalTransform = nodeTransform * boneOffset;
					const auto boneId = m_animeData.boneNameToId[node->mName.C_Str()];
					// DirectX Math로 변환
					DirectX::XMFLOAT3 bonePosition;
					bonePosition.x = globalTransform.a4;
					bonePosition.y = globalTransform.b4;
					bonePosition.z = globalTransform.c4;

					// 뼈 위치 추가
					m_animeData.bonePositions[boneId] = bonePosition;
				}
			}
		}


		// 자식 노드들 처리
		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			ExtractBonePositions(node->mChildren[i], nodeTransform, scene);
		}
	}
}

