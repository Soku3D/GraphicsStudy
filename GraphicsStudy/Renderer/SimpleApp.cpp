#include "SimpleApp.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    return Renderer::SimpleApp::m_pApp->MainProc(hWnd, message, wParam, lParam);
}
Renderer::SimpleApp* Renderer::SimpleApp::m_pApp = nullptr;

Renderer::SimpleApp::SimpleApp(const int& width, const int& height):
    m_screenWidth(width),
    m_screenHeight(height)  
{
    assert(m_pApp == nullptr);
    m_pApp = this;
}

Renderer::SimpleApp::~SimpleApp()
{
    m_pApp = nullptr;
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

    return m_mainWnd!=NULL;
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
    wcex.hCursor = NULL;
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
        rc.right - rc.left, 
        rc.bottom - rc.top, 
        nullptr,
        nullptr, 
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

    ShowWindow(m_mainWnd,SW_SHOWDEFAULT);
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
            double delTime = m_timer.GetDeltaTime();
            
            Update(delTime);
            Render(delTime);
            //std::cout << delTime << ' ' << elapsedTime << '\n';
        }
    }
    return (int)msg.wParam;
}

LRESULT Renderer::SimpleApp::MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        break;

    case WM_SIZE:
        {
            m_screenWidth = LOWORD(lParam);
            m_screenHeight = HIWORD(lParam);

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
        // if (raw.header.dwType == RIM_TYPEMOUSE && m_FPSMode) {
        int deltaX = raw.data.mouse.lLastX;
        int deltaY = raw.data.mouse.lLastY;
        std::cout << deltaX << " " << deltaY << '\n';
    }
    break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MOUSEWHEEL:
        {
            std::cout << "wParam : " << wParam << ", lParam : " << lParam << std::endl;
            return 0;
        }
        break;

   
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
    
}
