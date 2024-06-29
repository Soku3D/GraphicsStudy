#include "D3D12App.h"
#include "D3D11App.h"

int main() {

    try
    {
        Renderer::D3D11App m_app(1280, 720);
        if (!m_app.Initialize())
            return 0;

        return m_app.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }

}