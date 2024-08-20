#pragma once

#include "D3D12App.h"
#include "Renderer.h"
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
		virtual ~D3D12PassApp() {}

		bool Initialize() override;
		void InitConstantBuffers();
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;
		void CreateVertexAndIndexBuffer() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void GeometryPass(float& deltaTime);
		void FbxGeometryPass(float& deltaTime);
		void LightPass(float& deltaTime);
		void DrawNormalPass(float& deltaTime);
		void RenderCubeMap(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

		virtual void PostProcessing(float& deltaTime);

	protected:
		CSConstantData* psConstantData;
		UINT8* m_pCbufferBegin = nullptr;
		ComPtr<ID3D12Resource> m_csBuffer;

		CubeMapConstantData* m_pCubeMapConstantData;
		UINT8* m_pCubeMapCbufferBegin = nullptr;
		ComPtr<ID3D12Resource> m_cubeMapConstantBuffer;

	private:
		bool bRenderMeshes = true;
		bool bRenderFbx = true;
		bool bRenderNormal = true;

		float gui_cubeMapLod = 0.f;
		float gui_cubeMapExpose = 1.7f;
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

	private:
		const wchar_t* geomeytyPassEvent = L"Geometry Pass ";
		const wchar_t* fbxGeomeytyPassEvent = L"FBX Geometry Pass ";
		const wchar_t* lightPassEvent = L"Light Pass ";
		const wchar_t* drawNormalPassEvent = L"DrawNormal Pass ";
		const wchar_t* cubeMapPassEvent = L"CubeMap Pass ";

	};
}