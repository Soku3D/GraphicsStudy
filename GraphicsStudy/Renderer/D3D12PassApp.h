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
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;
		void CreateVertexAndIndexBuffer() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void GeometryPass(float& deltaTime);
		void LightPass(float& deltaTime);
		void DrawNormalPass(float& deltaTime);
		void RenderCubeMap(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

		void CopyResourceToSwapChain(float& deltaTime);

		virtual void PostProcessing(float& deltaTime);

	protected:
		CSConstantData* psConstantData;
		UINT8* m_pCbufferBegin = nullptr;
		ComPtr<ID3D12Resource> m_csBuffer;

	private:
		bool bRenderMeshes = true;
		bool bRenderFbx = true;
		bool bRenderNormal = true;
	};
}