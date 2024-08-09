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

		void RenderMeshes(float& deltaTime);

		void RenderCubeMap(float& deltaTime);

		void CreateCommandObjects();
		void CreateDescriptorHeaps();
		void FlushCommandQueue();

		void CreateVertexAndIndexBuffer();
		void RenderFonts(const std::wstring& output, std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors, std::shared_ptr<DirectX::SpriteBatch>& spriteBatch, std::shared_ptr<DirectX::SpriteFont>& font, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		void CreateConstantBuffer();
		void CreateTextures();
		void CreateCubeMapTextures();
		void CreateExrTexture();
		void CreateExrBuffer(std::wstring& path, ComPtr<ID3D12Resource>& upload, ComPtr<ID3D12Resource>& texture, UINT offset);
		//void CreateExrBuffer(std::wstring& path, ComPtr<ID3D12Resource>& upload, ComPtr<ID3D12Resource>& texture, UINT offset, std::vector<uint16_t>& image);
		void CreateFontFromFile(const std::wstring& fileName, std::shared_ptr<DirectX::SpriteFont>& font, std::shared_ptr<DirectX::SpriteBatch>& spriteBatch, std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors, bool bUseMsaa, DXGI_FORMAT& rtFormat, DXGI_FORMAT& dsFormat);

	protected:
		ID3D12Resource* CurrentBackBuffer() const;
		ID3D12Resource* MsaaRenderTargetBuffer() const;
		ID3D12Resource* HDRRenderTargetBuffer() const;
		
		D3D12_CPU_DESCRIPTOR_HANDLE MsaaDepthStencilView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE HDRDepthStencilView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE HDRUnorderedAccesslView() const;

		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE MsaaRenderTargetView() const;
		
		D3D12_CPU_DESCRIPTOR_HANDLE HDRRendertargetView() const;

		void ResolveSubresource(ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource* dest, ID3D12Resource* src);

		void CopyResource(ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource* dest, ID3D12Resource* src);

		void CreateDepthBuffer(ComPtr<ID3D12Resource>& buffer, D3D12_CPU_DESCRIPTOR_HANDLE& handle, bool bUseMsaa);
		void CreateRenderTargetBuffer(ComPtr<ID3D12Resource>& buffer, DXGI_FORMAT format, bool bUseMsaa, D3D12_RESOURCE_FLAGS flag);
		

	protected:
		bool bUseWarpAdapter;
		D3D_FEATURE_LEVEL m_minimumFeatureLevel;
		ComPtr<IDXGIFactory4> m_dxgiFactory;
		ComPtr<IDXGIAdapter1> m_warpAdapter;
		
		ComPtr<ID3D12Device> m_device;

		UINT m_numQualityLevels = 0;
		UINT m_sampleCount = 4;
		DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		DXGI_FORMAT m_msaaFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		ComPtr<IDXGISwapChain3> m_swapChain;
		static const UINT m_swapChainCount = 2;
		ComPtr<ID3D12Resource> m_renderTargets[m_swapChainCount];
		ComPtr<ID3D12Resource> m_msaaDepthStencilBuffer;
		UINT m_frameIndex = 0;

		UINT m_currentFence = 0;
		ComPtr<ID3D12Fence> m_fence;

		UINT m_rtvHeapSize = 0;
		UINT m_dsvHeapSize = 0;
		UINT m_csuHeapSize = 0;

		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
		ComPtr<ID3D12DescriptorHeap> m_textureHeap;
		ComPtr<ID3D12DescriptorHeap> m_cubeMapTextureHeap;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

		GlobalVertexConstantData* m_passConstantData;
		ComPtr<ID3D12Resource> m_passConstantBuffer;
		UINT8* m_pCbvDataBegin = nullptr;
		

		PSConstantData* m_psConstantData;
		ComPtr<ID3D12Resource> m_psConstantBuffer;
		UINT8* m_pPSDataBegin = nullptr;

		std::vector<std::shared_ptr<Core::StaticMesh>> m_staticMeshes;
		std::shared_ptr<Core::StaticMesh> m_cubeMap;

		std::wstring textureBasePath;
		std::wstring cubeMapTextureBasePath;
		std::wstring exrTextureBasePath;

		std::vector<ComPtr<ID3D12Resource>> m_textureResources;
		std::vector<ComPtr<ID3D12Resource>> m_cubeMaptextureResources;
		
		std::vector<ComPtr<ID3D12Resource>> m_exrUploadResources;
		std::vector<ComPtr<ID3D12Resource>> m_exrResources;
		ComPtr<ID3D12DescriptorHeap> m_exrSrvHeap;

		std::map<std::wstring, unsigned int> m_textureMap;

	protected:
		// GUI용 
		ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;
		
		std::shared_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
		std::shared_ptr<DirectX::DescriptorHeap> m_msaaResourceDescriptors;

		std::shared_ptr<DirectX::SpriteFont> m_font;
		std::shared_ptr<DirectX::SpriteFont> m_msaaFont;

		std::shared_ptr<DirectX::SpriteBatch> m_spriteBatch;
		std::shared_ptr<DirectX::SpriteBatch> m_msaaSpriteBatch;

		DirectX::SimpleMath::Vector2 m_fontPos;
		std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
		int m_textureNum = 0;

	protected:
		ComPtr<ID3D12Resource> m_msaaRenderTarget;

		ComPtr<ID3D12Resource> m_hdrRenderTarget;
		ComPtr<ID3D12Resource> m_hdrDepthStencilBuffer;

		ComPtr<ID3D12DescriptorHeap> m_msaaRtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_hdrRtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_hdrDepthHeap;
		ComPtr<ID3D12DescriptorHeap> m_hdrUavHeap;

	protected:
		std::string currRenderMode = "Msaa";
		DirectX::SimpleMath::Vector3 gui_lightPos;
		float gui_shineness;
		float gui_diffuse;
		float gui_specular;
	};
}