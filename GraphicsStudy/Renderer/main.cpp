#include "D3D12SimulationApp.h"
#include "D3D12PassApp.h"
#include "D3D12ComputeShaderApp.h"
#include "D3D12PhysxSimulationApp.h"
#include "D3D12RayTracingApp.h"

int main() {

    enum AppType {
        D3D12PassApp = 1,
        D3D12SimulationApp = 2,
        D3D12PhysxSimulationApp = 3,
        D3D12RayTracingApp = 4
    };

    auto currentApp = 3;

    try
    {
        Renderer::D3D12App* m_app = nullptr;
        switch (currentApp)
        {
        case AppType::D3D12PassApp:
            m_app = new Renderer::D3D12PassApp(1280, 720);
            break;
        case AppType::D3D12SimulationApp:
            m_app = new Renderer::D3D12SimulationApp(1280, 1280);
            break;
        case AppType::D3D12PhysxSimulationApp:
            m_app = new Renderer::D3D12PhysxSimulationApp(1280, 720);
            break;
        case AppType::D3D12RayTracingApp:
            m_app = new Renderer::D3D12RayTracingApp(1280, 720);
            break;
        }

        if (m_app && !m_app->Initialize())
            return 0;

        int wParam =  m_app->Run();
        delete m_app;
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }

}