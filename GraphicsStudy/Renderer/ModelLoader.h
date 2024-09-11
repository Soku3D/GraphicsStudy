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
			
			ProcessNode<Vertex, Index>(pScene->mRootNode, pScene, tr);

			m_animeData.offsetMatrices.resize(m_animeData.meshNameToId.size());
			m_animeData.meshTransforms.resize(m_animeData.meshNameToId.size());

			for (auto& mesh : m_animeData.meshNameToId)
			{
				std::cout << mesh.first << " : " << mesh.second << "\n";
			}
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
				clip.keys.resize(m_animeData.meshNameToId.size());
				clip.numChannels = ani->mNumChannels;

				// i번쨰 노드(채널 / 메쉬)
				for (uint32_t c = 0; c < ani->mNumChannels; c++) {
					const aiNodeAnim* nodeAnim = ani->mChannels[c];
					const int meshId =
						m_animeData.meshNameToId[nodeAnim->mNodeName.C_Str()];

					clip.keys[meshId].resize(nodeAnim->mNumPositionKeys);
					for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++) {
						const auto pos = nodeAnim->mPositionKeys[k].mValue;
						const auto rot = nodeAnim->mRotationKeys[k].mValue;
						const auto scale = nodeAnim->mScalingKeys[k].mValue;
						auto& key = clip.keys[meshId][k];
						key.pos = { pos.x, pos.y, pos.z };
						key.rot = Quaternion(rot.x, rot.y, rot.z, rot.w);
						key.scale = { scale.x, scale.y, scale.z };
					}
				}
			}
		}

		template<typename Vertex, typename Index>
		void ProcessNode(aiNode* node, const aiScene* scene,
			DirectX::SimpleMath::Matrix tr)
		{
			using DirectX::SimpleMath::Matrix;

			Matrix m;
			ai_real* temp = &node->mTransformation.a1;
			float* mTemp = &m._11;
			for (int t = 0; t < 16; t++) {
				mTemp[t] = float(temp[t]);
			}
			m = m.Transpose() * tr;

			for (UINT i = 0; i < node->mNumMeshes; i++) {

				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				m_animeData.meshNameToId[mesh->mName.C_Str()] = meshCount++;
				auto newMesh = this->ProcessMesh<Vertex, Index>(mesh, scene);
				newMesh.m_name = mesh->mName.C_Str();

				for (auto& v : newMesh.m_vertices) {
					v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
				}

				meshes.push_back(newMesh);
			}

			for (UINT i = 0; i < node->mNumChildren; i++) {
				this->ProcessNode<Vertex,Index>(node->mChildren[i], scene, m);
			}
		}

		template<typename Vertex, typename Index>
		MeshData<Vertex, Index> ProcessMesh(aiMesh* mesh, const aiScene* scene) {
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
					vertex.normal.x = mesh->mNormals[i].x;
					vertex.normal.y = mesh->mNormals[i].y;
					vertex.normal.z = mesh->mNormals[i].z;
					DirectX::SimpleMath::Vector3 n = vertex.normal;
					n.Normalize();
					vertex.normal = n;
				}
				else {
					//std::cout << "NULL Normals" << std::endl;
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
}

