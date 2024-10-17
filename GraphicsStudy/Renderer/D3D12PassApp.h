#pragma once

#include "D3D12App.h"
#include "Renderer.h"
#include "SkeletonMesh.h"
#include <DirectXTexEXR.h>

namespace Renderer {

	enum GeometryPassType {
		position,
		normal,
		albedoColor,
		specularColor
	};

	class D3D12PassApp :public D3D12App {
	public:
		D3D12PassApp(const int& width, const int& height);
		virtual ~D3D12PassApp() {
			if(skeletonMesh!=nullptr)
				delete skeletonMesh;
			std::cout << "~D3D12PassApp" << std::endl;
		}

		bool Initialize() override;
		virtual void InitConstantBuffers();
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;
		void InitScene() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void GeometryPass(float& deltaTime);
		void FbxGeometryPass(float& deltaTime);
		void SkinnedMeshGeometryPass(float& deltaTime);
		void LightPass(float& deltaTime);
		void RenderNormalPass(float& deltaTime);
		void RenderBoundingBoxPass(float& deltaTime);
		void RenderSkeleton(float& deltaTime);
		void RenderCubeMap(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

		void RenderStaticMeshes(float& deltaTime);
		void RenderLightMeshes(float& deltaTime);
		void RenderCharacter(float& deltaTime);
		void RenderPlayers(float& deltaTime);

		//virtual void PostProcessing(float& deltaTime);

	protected:
		Core::ConstantBuffer<CubeMapConstantData> mCubeMapConstantData;
		Core::ConstantBuffer<SkinnedMeshConstantData> mSkinnedMeshConstantData;
		
	protected:
		bool bRenderLights = true;
		bool bRenderMeshes = true;
		bool bRenderFbx = true;
		bool bRenderFPS = true;
		bool bRenderNormal = true;
		bool bUseDLAA = false;
		bool bRenderBoundingBox = false;
		bool bSkeleton = false;
		bool bRunAnimation = false;

		bool guiUseDLAA = true;

		float gui_cubeMapLod = 0.f;
		float gui_cubeMapExpose = 2.0f;
		DirectX::SimpleMath::Vector3 gui_lightPos;
		float gui_shineness;
		float gui_diffuse;
		float gui_specular;

		float gui_lod = 0.f;

		float gui_ao = 1.f;
		float gui_metallic = 0.f;
		float gui_roughness = 0.f;

		float gui_edge0 = 1.f;
		float gui_edge1 = 1.f;
		float gui_edge2 = 1.f;
		float gui_edge3 = 1.f;
		float gui_inside0 = 1.f;
		float gui_inside1 = 1.f;

		float gui_headX = 0.f;
		float gui_headY = 0.f;
		float gui_headZ = 0.f;

		float gui_jawZ = 0.f;
		float gui_jawRotation = 0.f;
		float gui_eyeRotation = 0.f;
		DirectX::SimpleMath::Matrix leftEye;
		DirectX::SimpleMath::Matrix jaw;
		DirectX::SimpleMath::Matrix head;
		Material gui_material;

		Core::SkeletonMesh* skeletonMesh;

	private:
		const wchar_t* geomeytyPassEvent = L"Geometry Pass ";
		const wchar_t* fbxGeomeytyPassEvent = L"FBX Geometry Pass ";
		const wchar_t* lightPassEvent = L"Light Pass ";
		

	protected:
		
		DirectX::BoundingSphere skeletonBoundingSphere;
		std::string skeletonName = "";
		int selectedSkeletonId = -1;
		float gui_skeletonX = 0.f;
		float gui_skeletonY = 0.f;
		float gui_skeletonZ = 0.f;
	};
}