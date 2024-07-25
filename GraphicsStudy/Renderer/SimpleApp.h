#pragma once

#include "Object.h"
#include "InputHandler.h"

namespace Renderer {
	class SimpleApp {
	public:
		SimpleApp(const int& width, const int& height);
		virtual ~SimpleApp();

		virtual bool Initialize();
		virtual bool InitDirectX() = 0;
		virtual void OnResize() = 0;

		bool InitWindow();
		bool InitbackgroundWindow();

		int Run();
		LRESULT MainProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void Render(float& deltaTime) = 0;
		virtual void Update(float& deltaTime) = 0;
	public:
		HWND m_mainWnd;
		static SimpleApp* m_pApp;
		Utils::Timer m_timer;
	public:
		UINT m_screenWidth;
		UINT m_screenHeight;
		
		std::shared_ptr<Camera> m_camera;
		std::unique_ptr<InputHandler> m_inputHandler;
	};
}