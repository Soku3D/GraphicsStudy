#pragma once

#include <map>

#include "SimpleApp.h"
#include "Constants.h"
#include "StaticMesh.h"

#include "directxtk12/DescriptorHeap.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/ResourceUploadBatch.h"
#include "directxtk12/GraphicsMemory.h"

using Microsoft::WRL::ComPtr;

namespace Renderer {
	enum Descriptors
	{
		MyFont,
		Count

	};
	class D3D12App :public SimpleApp {
	public:
		D3D12App(const int& width, const int& height);
		virtual ~D3D12App();

		virtual bool Initialize();
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;
		
		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

		void CreateCommandObjects();
		void CreateDescriptorHeaps();
		void FlushCommandQueue();

		void CreateVertexAndIndexBuffer();
		void RenderFonts(const std::wstring& output, std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors);
		void CreateConstantBuffer();
		void CreateTextures();
		void CreateFontFromFile(const std::wstring& fileName, std::shared_ptr<DirectX::SpriteFont>& font, std::shared_ptr<DirectX::SpriteBatch>& spriteBatch, std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors);

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetMSAARtV() const;

	protected:
		bool bUseWarpAdapter;
		D3D_FEATURE_LEVEL m_minimumFeatureLevel;
		ComPtr<IDXGIFactory4> m_dxgiFactory;
		ComPtr<IDXGIAdapter1> m_warpAdapter;
		
		ComPtr<ID3D12Device> m_device;

		UINT m_numQualityLevels = 0;
		UINT m_sampleCount = 4;
		DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		ComPtr<IDXGISwapChain3> m_swapChain;
		static const UINT m_swapChainCount = 2;
		ComPtr<ID3D12Resource> m_renderTargets[m_swapChainCount];
		ComPtr<ID3D12Resource> m_depthStencilBuffer;
		UINT m_frameIndex = 0;

		UINT m_currentFence = 0;
		ComPtr<ID3D12Fence> m_fence;

		UINT m_rtvHeapSize = 0;
		UINT m_dsvHeapSize = 0;
		UINT m_csuHeapSize = 0;

		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

		GlobalVertexConstantData* m_passConstantData;
		ComPtr<ID3D12Resource> m_passConstantBuffer;
		UINT8* m_pCbvDataBegin = nullptr;
		
		ComPtr<ID3D12Resource> m_uploadHeap;
		std::vector<std::shared_ptr<Core::StaticMesh>> m_staticMeshes;

		std::wstring textureBasePath;
		std::vector<ComPtr<ID3D12Resource>> m_textureResources;
		std::map<std::wstring, unsigned int> m_textureMap;

	protected:
		std::shared_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
		std::shared_ptr<DirectX::DescriptorHeap> m_guiResourceDescriptors;
		
		ComPtr<ID3D12DescriptorHeap> m_fontHeap;

		std::shared_ptr<DirectX::SpriteFont> m_font;
		std::shared_ptr<DirectX::SpriteFont> m_guiFont;
		std::shared_ptr<DirectX::SpriteBatch> m_spriteBatch;
		std::shared_ptr<DirectX::SpriteBatch> m_guiSpriteBatch;

		DirectX::SimpleMath::Vector2 m_fontPos;
		std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
		int m_textureNum = 0;

	private:
		ComPtr<ID3D12Resource> m_msaaRenderTarget;
		ComPtr<ID3D12DescriptorHeap> m_msaaRtvHeap;

	private:
		float test = 0.f;
		std::string currRenderMode = "Default";

	};
}