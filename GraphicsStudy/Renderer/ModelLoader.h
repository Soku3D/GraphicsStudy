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

    class ModelLoader {
    public:
        ModelLoader();
        ~ModelLoader();
        void Load(std::string filename, bool loadAnimation = false);
        void ReadAnimation(const aiScene* pScene);
        //void Load(std::string basePath, std::string filename);

        void ProcessNode(aiNode* node, const aiScene* scene,
            DirectX::SimpleMath::Matrix tr);
        
        Animation::AnimationData m_animeData;
        BasicMeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);
        int meshCount = 0;
    public:
        std::string basePath;
        std::vector<BasicMeshData> meshes;
    };
}
