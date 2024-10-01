#include "SimpleApp.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return Renderer::SimpleApp::m_pApp->MainProc(hWnd, message, wParam, lParam);
}
Renderer::SimpleApp* Renderer::SimpleApp::m_pApp = nullptr;

Renderer::SimpleApp::SimpleApp(const int& width, const int& height) :
	m_screenWidth(width),
	m_screenHeight(height)
{
	assert(m_pApp == nullptr);
	m_pApp = this;
	m_camera = std::make_unique<Core::Camera>();
	m_inputHandler = std::make_unique<InputHandler>();
	mCharacter = std::make_unique<Core::Character>();

	m_camera->SetAspectRatio(width / (float)height);
	mCharacter->SetCameraAspectRatio(width / (float)height);
}

Renderer::SimpleApp::~SimpleApp()
{
	std::cout << "~SimpleApp" << std::endl;
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	
	m_mainWnd = NULL;
	m_pApp = nullptr;
	//m_camera.reset();
	//mCharacter.reset();
}

bool Renderer::SimpleApp::Initialize()
{
	if (!InitWindow()) {
		std::cerr << "Failed InitWindow()\n";
		return false;
	}
	/*if (!InitbackgroundWindow()) {
		 std::cerr << "Failed InitWindow()\n";
		 return false;
	}*/
	if (!InitDirectX()) {
		std::cerr << "Failed InitDirectX()\n";
		return false;
	}
	OnResize();

	return true;
}

bool Renderer::SimpleApp::InitbackgroundWindow()
{
	HWND progman = FindWindow(L"Progman", NULL);

	if (progman)
	{
		// Progman을 활성화시킴
		SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
		// WorkerW 찾기
		EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
			HWND p = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
			if (p)
			{
				HWND* ret = (HWND*)lParam;
				*ret = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
			}
			return TRUE;
			}, (LPARAM)&m_mainWnd);
	}
	assert(m_mainWnd != 0);

	RAWINPUTDEVICE rawInputDevice;

	//The HID standard for mouse
	const uint16_t standardMouse = 0x02;

	rawInputDevice.usUsagePage = 0x01;
	rawInputDevice.usUsage = standardMouse;
	rawInputDevice.dwFlags = 0;
	rawInputDevice.hwndTarget = m_mainWnd;

	::RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE));

	ShowWindow(m_mainWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_mainWnd);

	return true;
}

bool Renderer::SimpleApp::InitWindow()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"D3DApp";
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	// Create window
	RECT rc = { 0, 0, (LONG)m_screenWidth, (LONG)m_screenHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	m_mainWnd = CreateWindow(wcex.lpszClassName, wcex.lpszClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(int)(rc.right - rc.left),
		(int)(rc.bottom - rc.top),
		NULL,
		NULL,
		wcex.hInstance,
		nullptr);

	assert(m_mainWnd != 0);

	RAWINPUTDEVICE rawInputDevice;

	//The HID standard for mouse
	const uint16_t standardMouse = 0x02;

	rawInputDevice.usUsagePage = 0x01;
	rawInputDevice.usUsage = standardMouse;
	rawInputDevice.dwFlags = 0;
	rawInputDevice.hwndTarget = m_mainWnd;

	::RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE));

	ShowWindow(m_mainWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_mainWnd);

	return true;
}

int Renderer::SimpleApp::Run()
{
	MSG msg = {};
	m_timer.Start();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_timer.Tick();
			
			float delTime = min((float)m_timer.GetDeltaTime(), 1 / 60.f);
			Update(delTime);
			Render(delTime);
			RenderGUI(delTime);

			//std::cout << delTime << ' ' << elapsedTime << '\n';
		}
	}

	std::cout << "QUIT" << std::endl;
	return (int)msg.wParam;
}

LRESULT Renderer::SimpleApp::MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE:
	{
		m_screenWidth = LOWORD(lParam);
		m_screenHeight = HIWORD(lParam);
		if (m_camera != nullptr)
		{
			m_camera->SetAspectRatio(m_screenWidth / (float)m_screenHeight);
			mCharacter->SetCameraAspectRatio(m_screenWidth / (float)m_screenHeight);
		}
		OnResize();

		return 0;
	}
	break;
	case WM_INPUT:
	{
		RAWINPUT raw;
		UINT rawSize = sizeof(raw);

		const UINT resultData =
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
				&raw, &rawSize, sizeof(RAWINPUTHEADER));

		mouseDeltaX = raw.data.mouse.lLastX;
		mouseDeltaY = raw.data.mouse.lLastY;
		

		if (raw.data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN) {
			if (!bIsFPSMode) {
				GetCursorPos(&m_fpsModeCursorPos);
				//std::cout << m_fpsModeCursorPos.x << " " << m_fpsModeCursorPos.y << std::endl;
				bIsFPSMode = true;
				ShowCursor(FALSE);
			}
		}
		else if (raw.data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_UP) {
			bIsFPSMode = false;
			ShowCursor(TRUE);
			SetCursorPos(m_fpsModeCursorPos.x, m_fpsModeCursorPos.y);
		}
		if (raw.data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN) {
			
			if (!lMouseButtonClicked) {
				lMouseButtonClicked = true;
				fire = true;
			}
			else {
				fire = false;
			}
		}
		else if (raw.data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_UP) {
			lMouseButtonClicked = false;
		}
		if (m_camera != nullptr && bIsFPSMode)
		{
			m_camera->SetRotation(mouseDeltaX, mouseDeltaY);
			mCharacter->SetRotation(mouseDeltaX, mouseDeltaY);
			// m_camera->SetQuaternion(deltaX, deltaY);
		}

		POINT cursorPos;
		GetCursorPos(&cursorPos);
		RECT windowRect;
		GetWindowRect(hWnd, &windowRect);

		if (cursorPos.x < windowRect.left || cursorPos.x > windowRect.right ||
			cursorPos.y < windowRect.top || cursorPos.y > windowRect.bottom)
		{
			// 화면 밖으로 나갔다면 버튼 상태를 해제
			lMouseButtonClicked = false;
			fire = false;
		}

		return 0;
	}
	break;

	case WM_KEYDOWN:
		m_inputHandler->m_currKeyStates[(int)wParam] = true;

		break;

	case WM_KEYUP:
		m_inputHandler->m_currKeyStates[(int)wParam] = false;
		if (wParam == 'C') {
			if (m_appName == "SimulationApp" && bCaptureBackbuffer) {
				CaptureBackBufferToPNG();
			}
			else
				CaptureHDRBufferToPNG();
		}
		//m_inputHandler->UpdateKeyUp((int)wParam);

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);

}
