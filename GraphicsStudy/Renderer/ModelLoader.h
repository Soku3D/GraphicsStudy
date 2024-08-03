#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "MeshData.h"
#include "Vertex.h"

namespace Renderer {

    class ModelLoader {
    public:
        ModelLoader();
        void Load(std::string filename);
        //void Load(std::string basePath, std::string filename);

        void ProcessNode(aiNode* node, const aiScene* scene,
            DirectX::SimpleMath::Matrix tr);

        BasicMeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);

    public:
        std::string basePath;
        std::vector<BasicMeshData> meshes;
    };
}
