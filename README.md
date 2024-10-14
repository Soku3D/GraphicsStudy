# GraphicsStudy


### DirectX 12 samples
1. Deferred Shading sample
    
    <img src="GraphicsStudy/Renderer/Results/PassApp/240828-2342.png" width="640" height="360">
    <img src="GraphicsStudy/Renderer/Results/PassApp/241007-1305.png" width="640" height="360">
    <img src="GraphicsStudy/Renderer/Results/PassApp/241009-2242.png" width="640" height="360">

2. Physx Simulation sample
    
    <img src="GraphicsStudy/Renderer/Results/PhysxSimulationApp/240828-1952.png" alt="Physx Simulation" height="200">

3. Raytracing sample
    
    <img src="GraphicsStudy/Renderer/Results/RaytracingApp/240911-2317.png" height="200">
    <img src="GraphicsStudy/Renderer/Results/RaytracingApp/240916-1228.png" height="200">
    <img src="GraphicsStudy/Renderer/Results/RaytracingApp/240922-2242.png" height="200">
    
4. Simulation sample
    
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240905-2056.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240914-2159.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240914-2333.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240918-1725.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240920-959.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240920-2132.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/240920-2130.png" width="180" height="180">
    <img src="GraphicsStudy/Renderer/Results/SimulationApp/241003-1800.png" height="180">

5. DLSS sample

    <img src="GraphicsStudy/Renderer/Results/PhysxSimulationApp/before.png" height="180">
    <img src="GraphicsStudy/Renderer/Results/PhysxSimulationApp/after.png" height="180">


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
```

#### Set Up Streamline SDK

1. Visit the [Streamline SDK Github](https://github.com/NVIDIAGameWorks/Streamline).
2. Download the Streamline SDK from the website.
3. Once downloaded, extract the SDK files to the following path:

```
C:\Users\username\streamline
```
