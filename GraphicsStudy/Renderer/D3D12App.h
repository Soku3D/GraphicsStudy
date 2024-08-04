#pragma once

#include "SimpleApp.h"
#include "Constants.h"
#include "StaticMesh.h"
#include <map>

#include "directxtk12/DescriptorHeap.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/ResourceUploadBatch.h"

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
		virtual bool InitDirectX() override;
		virtual void OnResize() override;
		
		virtual void Update(float& deltaTime) override;
		virtual void Render(float& deltaTime) override;
					
		void CreateCommandObjects();
		void CreateDescriptorHeaps();
		void FlushCommandQueue();

		void CreateVertexAndIndexBuffer();
		void CreateConstantBuffer();
		void CreateRootSignature();
		void CreatePSO();

		void CreateTextures();

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

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

		D3D12_COMMAND_LIST_TYPE m_commandType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		
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

		ComPtr<ID3D12PipelineState> m_simplePso;
		ComPtr<ID3D12PipelineState> m_pso;
		ComPtr<ID3D12PipelineState> m_wireModePso;
		ComPtr<ID3D12RootSignature> m_rootSignature;

		GlobalVertexConstantData* m_passConstantData;
		ComPtr<ID3D12Resource> m_passConstantBuffer;
		UINT8* m_pCbvDataBegin = nullptr;

		D3D12_INPUT_LAYOUT_DESC m_simpleVertexInputLayout;
		D3D12_INPUT_LAYOUT_DESC m_vertexInputLayout;

		ComPtr<ID3D12Resource> m_uploadHeap;

		std::vector<std::shared_ptr<Core::StaticMesh>> m_staticMeshes;

		D3D12_RASTERIZER_DESC wireFrameRasterizer;

		std::wstring textureBasePath;
		std::vector<ComPtr<ID3D12Resource>> m_textureResources;
		std::map<std::wstring, unsigned int> m_textureMap;

	private:
		std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
		std::unique_ptr<DirectX::SpriteFont> m_font;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	};
}