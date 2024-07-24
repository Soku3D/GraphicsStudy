#pragma once

#include "Object.h"

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

		virtual void Render(const double& deltaTime) = 0;
		virtual void Update(const double& deltaTime) = 0;
	public:
		HWND m_mainWnd;
		static SimpleApp* m_pApp;
		Utils::Timer m_timer;
	public:
		UINT m_screenWidth;
		UINT m_screenHeight;
		
		std::unique_ptr<Camera> m_camera;
	};
}