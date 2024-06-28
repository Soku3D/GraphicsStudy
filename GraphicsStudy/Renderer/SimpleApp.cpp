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
    if (!InitDirectX()) {
        std::cerr << "Failed InitDirectX()\n";
        return false;
    }
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
            Update(m_timer.GetDeltaTime());
            Render(m_timer.GetDeltaTime());
        }
    }
    return (int)msg.wParam;
}

LRESULT Renderer::SimpleApp::MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        OnResize();
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}