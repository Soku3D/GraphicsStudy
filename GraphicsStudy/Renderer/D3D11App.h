#pragma once

#include "SimpleApp.h"

namespace Renderer {
	class D3D11App : public SimpleApp {
	public:
		D3D11App(const int& width, const int& height);
		virtual ~D3D11App();

		virtual bool Initialize();
		virtual bool InitDirectX() override;
		virtual void OnResize() override;

		void SetViewport();
		void CreateRenderTargets();
		void CreateDepthBuffer();
		void CreateRaseterizerState();

		virtual void Update(const double& deltaTime) override;
		virtual void Render(const double& deltaTime) override;
	protected:
		UINT m_msaaQuality;
		bool m_useMsaa = true;
		UINT m_sampleCount = 4;
	private:
		DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_VIEWPORT m_viewport;
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	};
}