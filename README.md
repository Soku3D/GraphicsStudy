# GraphicsStudy


### DirectX 12 samples
1. Deferred Shading sample
    
    <img src="GraphicsStudy/Renderer/Results/PassApp/240828-2342.png" height="200">
    <img src="GraphicsStudy/Renderer/Results/PassApp/240915-1314.png" height="200">

1. Physx Simulation sample
    
    <img src="GraphicsStudy/Renderer/Results/PhysxSimulationApp/240828-1952.png" alt="Physx Simulation" height="200">

1. Raytracing sample
    
    <img src="GraphicsStudy/Renderer/Results/RaytracingApp/240911-2317.png" height="200">
    <img src="GraphicsStudy/Renderer/Results/RaytracingApp/240828-2337.png" height="200">
    <img src="GraphicsStudy/Renderer/Results/RaytracingApp/240916-1228.png" height="200">

1. Simulation sample
    
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240905-2056.png" width="200" height="200">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240914-2159.png" width="200" height="200">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240914-2333.png" width="200" height="200">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240918-1725.png" width="200" height="200">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240920-959.png" width="200" height="200">

### Windows
#### Installing Dependencies
```
vcpkg install directxtex[core,dx11,openexr]:x64-windows
vcpkg install directxtk12[core,xaudio2-9]:x64-windows
vcpkg install directxtk[core,xaudio2-9]:x64-windows
vcpkg install fp16:x64-windows glm:x64-windows
vcpkg install imgui[core,dx11-binding,dx12-binding,win32-binding]:x64-windows
vcpkg install assimp:x64-windows
vcpkg install boost-serialization:x64-windows
vcpkg install physx:x64-windows
```
#### Set Up Steam SDK

1. Visit the [Steamworks SDK Documentation](https://partner.steamgames.com/doc/sdk).
2. Download the Steam SDK from the website.
3. Once downloaded, extract the SDK files to the following path:

```
C:\Users\username\sdk
