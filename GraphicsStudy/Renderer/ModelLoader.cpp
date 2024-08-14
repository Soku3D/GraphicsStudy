#include "ModelLoader.h"

#include <filesystem>

namespace Renderer {
    using namespace DirectX::SimpleMath;

    ModelLoader::ModelLoader()
    {
        basePath = std::filesystem::current_path().string();
        basePath += "/Models/";
    }

    void ModelLoader::Load(std::string filename, bool loadAnimation) {

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
        Matrix tr; // Initial transformation
        ProcessNode(pScene->mRootNode, pScene, tr);

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

        // 노멀 벡터가 없는 경우를 대비하여 다시 계산
        // 한 위치에는 한 버텍스만 있어야 연결 관계를 찾을 수 있음
        /* for (auto &m : this->meshes) {

            vector<Vector3> normalsTemp(m.vertices.size(), Vector3(0.0f));
            vector<float> weightsTemp(m.vertices.size(), 0.0f);

            for (int i = 0; i < m.indices.size(); i += 3) {

                int idx0 = m.indices[i];
                int idx1 = m.indices[i + 1];
                int idx2 = m.indices[i + 2];

                auto v0 = m.vertices[idx0];
                auto v1 = m.vertices[idx1];
                auto v2 = m.vertices[idx2];

                auto faceNormal =
                    (v1.position - v0.position).Cross(v2.position - v0.position);

                normalsTemp[idx0] += faceNormal;
                normalsTemp[idx1] += faceNormal;
                normalsTemp[idx2] += faceNormal;
                weightsTemp[idx0] += 1.0f;
                weightsTemp[idx1] += 1.0f;
                weightsTemp[idx2] += 1.0f;
            }

            for (int i = 0; i < m.vertices.size(); i++) {
                if (weightsTemp[i] > 0.0f) {
                    m.vertices[i].normal = normalsTemp[i] / weightsTemp[i];
                    m.vertices[i].normal.Normalize();
                }
            }
        }*/
    }

    void ModelLoader::ReadAnimation(const aiScene* pScene) {

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

    void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, Matrix tr) {
        static int meshCount = 0;

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
            auto newMesh = this->ProcessMesh(mesh, scene);
            newMesh.m_name = mesh->mName.C_Str();

            for (auto& v : newMesh.m_vertices) {
                v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
            }

            meshes.push_back(newMesh);
        }

        for (UINT i = 0; i < node->mNumChildren; i++) {
            this->ProcessNode(node->mChildren[i], scene, m);
        }
    }

    BasicMeshData ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        // Data to fill
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

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

                vertex.normal.Normalize();
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

        BasicMeshData newMesh;
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
}