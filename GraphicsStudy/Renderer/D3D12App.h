#pragma once

#include "SimpleApp.h"


using Microsoft::WRL::ComPtr;

namespace Renderer {
	class D3D12App :public SimpleApp {
	public:
		D3D12App(const int& width, const int& height);
		virtual ~D3D12App();

		virtual bool Initialize();
		virtual bool InitDirectX() override;
		virtual void OnResize() override;
		
		virtual void Update(const double& deltaTime) override;
		virtual void Render(const double& deltaTime) override;

		void FlushCommandQueue();

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		ID3D12Resource* CurrentBackBuffer() const;
	private:
		bool bUseWarpAdapter;
		D3D_FEATURE_LEVEL m_minimumFeatureLevel;
		ComPtr<IDXGIFactory4> m_dxgiFactory;
		ComPtr<IDXGIAdapter1> m_warpAdapter;
		
		ComPtr<ID3D12Device> m_device;

		UINT m_numQualityLevels = 0;
		UINT m_sampleCount = 4;
		DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_COMMAND_LIST_TYPE m_commandType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12CommandQueue> m_commandQueue;

		ComPtr<IDXGISwapChain3> m_swapChain;
		static const UINT m_swapChainCount = 2;
		ComPtr<ID3D12Resource> m_renderTargets[m_swapChainCount];
		UINT m_frameIndex = 0;

		UINT m_currentFence = 0;
		ComPtr<ID3D12Fence> m_fence;

		UINT m_rtvHeapSize = 0;
		UINT m_dsvHeapSize = 0;
		UINT m_csuHeapSize = 0;

		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

		ComPtr<ID3D12Resource> m_vertexUpload;
		ComPtr<ID3D12Resource> m_vertexGpu;
		D3D12_VERTEX_BUFFER_VIEW vbv;

		ComPtr<ID3D12Resource> m_indexUpload;
		ComPtr<ID3D12Resource> m_indexGpu;
		D3D12_INDEX_BUFFER_VIEW ibv;

		ComPtr<ID3D12PipelineState> m_pso;
		ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}