#pragma once

namespace Renderer {
	class D3D12App;
}

#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
#include "SimpleApp.h"
#include "Constants.h"
#include "StaticMesh.h"
#include "FBX.h"

#include "directxtk12/DescriptorHeap.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/ResourceUploadBatch.h"
#include "directxtk12/GraphicsMemory.h"
#include "pix3.h"

#include "Buffer.h"
#include "SteamOnlineSystem.h"

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

		void InitSoundEngine();

		virtual bool Initialize();
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;
		
		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

		void RenderMeshes(float& deltaTime);

		virtual void RenderCubeMap(float& deltaTime);

		void CreateCommandObjects();
		void CreateDescriptorHeaps();

		void FlushCommandList(ComPtr<ID3D12GraphicsCommandList>& commandList);

		void FlushCommandQueue();

		virtual void InitScene();
		void RenderFonts(const std::wstring& output, std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors, std::shared_ptr<DirectX::SpriteBatch>& spriteBatch, std::shared_ptr<DirectX::SpriteFont>& font, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		virtual void CreateConstantBuffer();
		void CreateTextures();
		void CreateCubeMapTextures();
		void CreateExrTexture();
		void CreateExrBuffer(std::wstring& path, ComPtr<ID3D12Resource>& upload, ComPtr<ID3D12Resource>& texture, UINT offset);
		void CreateCubeMapBuffer(std::wstring& path, ComPtr<ID3D12Resource>& upload, ComPtr<ID3D12Resource>& texture, UINT offset);
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

		void ResolveSubresource(ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource* dest, ID3D12Resource* src,
			D3D12_RESOURCE_STATES destState = D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATES srcState = D3D12_RESOURCE_STATE_RENDER_TARGET,
			DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT);

		void CopyResource(ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource* dest, ID3D12Resource* src,
			D3D12_RESOURCE_STATES destState = D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATES srcState = D3D12_RESOURCE_STATE_RENDER_TARGET);


		void CopyResourceToSwapChain(float& deltaTime);

		void CreateDepthBuffer(ComPtr<ID3D12Resource>& buffer, D3D12_CPU_DESCRIPTOR_HANDLE& handle, bool bUseMsaa);
		void CreateResourceBuffer(ComPtr<ID3D12Resource>& buffer, DXGI_FORMAT format, bool bUseMsaa, D3D12_RESOURCE_FLAGS flag, 
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET, bool bUseClear = true);
		void CreateResourceView(ComPtr<ID3D12Resource>& buffer, DXGI_FORMAT format, bool bUseMsaa, D3D12_CPU_DESCRIPTOR_HANDLE& handle, 
			ComPtr<ID3D12Device5>& deivce, const Renderer::DescriptorType& type);
		void CaptureBufferToPNG() override;
		/*void CreateDescriptorHeap(ComPtr<ID3D12Device>& deivce, ComPtr<ID3D12DescriptorHeap>& heap, const Renderer::DescriptorType& type, int Numdescriptors,
			D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);*/
		virtual void PostProcessing(float& deltaTime);

	protected:
		bool bUseWarpAdapter;
		D3D_FEATURE_LEVEL m_minimumFeatureLevel;
		ComPtr<IDXGIFactory4> m_dxgiFactory;
		ComPtr<IDXGIAdapter1> m_warpAdapter;
		
		ComPtr<ID3D12Device5> m_device;

		UINT m_numQualityLevels = 0;
		UINT m_sampleCount = 4;
		DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		//DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		DXGI_FORMAT m_hdrFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
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

		ComPtr<ID3D12DescriptorHeap> m_swapChainRtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_swapChainUavHeap;

		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
		ComPtr<ID3D12DescriptorHeap> m_textureHeap;
		ComPtr<ID3D12DescriptorHeap> m_textureHeapNSV;
		ComPtr<ID3D12DescriptorHeap> m_cubeMapTextureHeapNSV;
		ComPtr<ID3D12DescriptorHeap> m_cubeMapTextureHeap;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

		GlobalVertexConstantData* m_passConstantData;
		ComPtr<ID3D12Resource> m_passConstantBuffer;
		UINT8* m_pCbvDataBegin = nullptr;
		

		LightPassConstantData* m_ligthPassConstantData;
		ComPtr<ID3D12Resource> m_ligthPassConstantBuffer;
		UINT8* m_pLPCDataBegin = nullptr;

		std::vector<std::shared_ptr<Animation::FBX>> m_fbxList;

		std::vector<std::shared_ptr<Core::StaticMesh>> m_staticMeshes;
		
		std::shared_ptr<Core::StaticMesh> m_cubeMap;
		std::shared_ptr<Core::StaticMesh> m_screenMesh;

		std::wstring textureBasePath;
		std::wstring cubeMapTextureBasePath;
		std::wstring exrTextureBasePath;
		std::wstring soundBasePath;
		std::wstring soundPath;

		std::vector<ComPtr<ID3D12Resource>> m_textureResources;
		std::vector<ComPtr<ID3D12Resource>> m_uploadResources;
		std::vector<ComPtr<ID3D12Resource>> m_cubeMaptextureResources;
		std::vector<ComPtr<ID3D12Resource>> m_cubeMaptextureUpload;
		
		std::vector<ComPtr<ID3D12Resource>> m_exrUploadResources;
		std::vector<ComPtr<ID3D12Resource>> m_exrResources;
		ComPtr<ID3D12DescriptorHeap> m_exrSrvHeap;

		std::map<std::wstring, unsigned int> m_textureMap;
		std::map<std::wstring, unsigned int> m_cubeTextureMap;

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
		ComPtr<ID3D12DescriptorHeap> m_hdrUavHeapNSV;
		ComPtr<ID3D12DescriptorHeap> m_hdrSrvHeap;

	protected:
		std::string currRenderMode = "Default";
		bool msaaMode = false;
		
	protected:
		// GeometryPass
		static const UINT geometryPassRtvNum = 4;
		ComPtr<ID3D12DescriptorHeap> m_geometryPassRtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_geometryPassSrvHeap;
		ComPtr<ID3D12Resource> m_geometryPassResources[geometryPassRtvNum];

		ComPtr<ID3D12DescriptorHeap> m_geometryPassMsaaRtvHeap;
		ComPtr<ID3D12Resource> m_geometryPassMsaaResources[geometryPassRtvNum];

		DXGI_FORMAT m_geometryPassFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		//DXGI_FORMAT m_geometryPassFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		void CreateSamplers();

		D3D12_CPU_DESCRIPTOR_HANDLE GeometryPassRTV() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GeometryPassMsaaRTV() const;


		Core::ConstantBuffer<CSConstantData> mCsBuffer;
		const wchar_t* postprocessingEvent = L"Postprocessing Pass ";

	protected:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap;

	private:
		const wchar_t* copyResourceToSwapChainEvent = L"CopyResourceoSwapChain Pass ";
		const wchar_t* copyResourceEvent = L"CopyResource Pass ";
		const wchar_t* guiPassEvent = L"GUI Pass ";

	protected:
		bool bUseTextureApp = true;
		bool bUseCubeMapApp = true;
		bool bUseDefaultSceneApp = true;

	protected:
		std::vector<uint16_t> imagef16;
		std::vector<uint8_t> imageUnorm;
		ComPtr<ID3D12Resource> imageBuffer;

	protected:
		std::string m_appName = "D3D12App";

	protected:
		std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

		std::vector<std::unique_ptr<DirectX::SoundEffect>> m_soundEffects;
		std::map<std::string, uint8_t> soundMap;
		std::unique_ptr<SoundEffectInstance> effect;
		AudioListener listener;
		AudioEmitter emitter;

	protected:
		class Network::SteamOnlineSystem* onlineSystem;
		bool bIsHost = false;
		bool createSession = false;
		bool findSession = false;

		std::shared_ptr<Core::StaticMesh> mCharacter;
		
	public:
		std::vector<std::shared_ptr<Core::StaticMesh>> mPlayers;
		void AddPlayer();
		void UpdatePlayer(int index, DirectX::SimpleMath::Vector3& position);
	};
}