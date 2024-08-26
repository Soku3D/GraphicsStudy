#include "D3D12SimulationApp.h"
#include "D3D12PassApp.h"
#include "D3D12ComputeShaderApp.h"
#include "D3D12PhysxSimulationApp.h"
#include "D3D12RayTracingApp.h"

int main() {

    try
    {
        //Renderer::D3D12PassApp m_app(1280, 720);
        //Renderer::D3D12SimulationApp  m_app(1280, 720);
        //Renderer::D3D12PhysxSimulationApp  m_app(1280, 720);
        Renderer::D3D12RayTracingApp  m_app(1280, 720);
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