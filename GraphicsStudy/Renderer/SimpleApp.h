#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "Object.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Character.h"

namespace Renderer {
	class SimpleApp {
	public:
		SimpleApp(const int& width, const int& height);
		virtual ~SimpleApp();

		virtual bool Initialize();
		virtual bool InitDirectX() = 0;
		virtual bool InitGUI() = 0;
		virtual void OnResize() = 0;

		bool InitWindow();
		bool InitbackgroundWindow();

		int Run();
		LRESULT MainProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void RenderGUI(float& deltaTime) = 0;
		virtual void Render(float& deltaTime) = 0;

		virtual void UpdateGUI(float& deltaTime) = 0;
		virtual void Update(float& deltaTime) = 0;
		virtual void CaptureBufferToPNG() {};
		virtual void CaptureBackBufferToPNG() {};
	public:
		HWND m_mainWnd;
		static SimpleApp* m_pApp;
		Utils::Timer m_timer;
	public:
		UINT m_screenWidth;
		float m_prevGuiWidth = 0.f;
		float m_currGuiWidth = 0.f;

		UINT m_screenHeight;
		
		std::shared_ptr<Core::Camera> m_camera;
		std::unique_ptr<InputHandler> m_inputHandler;
		std::unique_ptr<Core::Character> mCharacter;
		
		bool bIsFPSMode = false;
		bool bUseGUI = true;
		bool bRenderCubeMap = true;
		bool lMouseButtonClicked = false;
		bool fire = false;
		POINT m_fpsModeCursorPos;
		POINT mCursorPosition;

	protected:
		D3D12_COMMAND_LIST_TYPE m_commandType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12CommandAllocator> m_guiCommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12GraphicsCommandList> m_guiCommandList;
		ComPtr<ID3D12CommandQueue> m_commandQueue;

		std::vector< ComPtr<ID3D12CommandQueue>> m_tempCommandQueue;

	protected:
		std::string m_appName = "SimpleApp";
		int mouseDeltaX = 0;
		int mouseDeltaY = 0;
	};
}