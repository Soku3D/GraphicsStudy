#pragma once

#include "D3D12App.h"
#include "Renderer.h"

#include "nvsdk_ngx.h"
#include "nvsdk_ngx_helpers.h"

namespace Renderer {

	class D3D12DLSSApp :public D3D12App {
	public:
		D3D12DLSSApp(const int& width, const int& height);
		virtual ~D3D12DLSSApp() {
		}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;

		void InitializeDLSS();
		bool CreateDLSSFeature(ID3D12Resource* inputResource, ID3D12Resource* outputResource, NVSDK_NGX_Handle** pHandle);
		void ApplyDLSS(NVSDK_NGX_Handle* handle, ID3D12GraphicsCommandList* commandList, ID3D12Resource* input, ID3D12Resource* output);
		void CleanupDLSS();
		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

	private:
		NVSDK_NGX_Handle* dlssHandle = nullptr;           // DLSS 핸들
		NVSDK_NGX_Parameter* dlssParameters = nullptr;
	};
	
}