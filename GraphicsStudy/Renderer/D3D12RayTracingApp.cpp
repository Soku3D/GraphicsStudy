#include "D3D12RaytracingApp.h"
#include "Raytracing.hlsl.h"

using namespace std;

const wchar_t* c_hitGroupName = L"MyHitGroup";
const wchar_t* c_raygenShaderName = L"MyRaygenShader";
const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* c_missShaderName = L"MyMissShader";

Renderer::D3D12RayTracingApp::D3D12RayTracingApp(const int& width, const int& height)
	: D3D12App(width, height)
{
	bUseTextureApp = false;
	bUseCubeMapApp = false;
	bUseDefaultSceneApp = false;
	bUseGUI = false;
}

bool Renderer::D3D12RayTracingApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	ThrowIfFailed(m_commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");

	m_commandAllocator->Reset();
	m_dxrCommandList->Reset(m_commandAllocator.Get(), nullptr);

	std::shared_ptr<Core::StaticMesh> tri = std::make_shared<Core::StaticMesh>();
	tri->Initialize(GeometryGenerator::SimpleTriangle(1.f), m_device, m_commandList);
	tri->BuildAccelerationStructures<SimpleVertex>(m_device, m_dxrCommandList);
	m_staticMeshes.push_back(tri);

	CD3DX12_DESCRIPTOR_RANGE1 rangeTable;
	std::vector<CD3DX12_ROOT_PARAMETER1> paramenters;
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	paramenters.resize(2);
	paramenters[0].InitAsDescriptorTable(1, &rangeTable);
	paramenters[1].InitAsShaderResourceView(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_globalRootSignature));

	D3D12_EXPORT_DESC raygenExport{
		L"RayGen",
		nullptr,
		D3D12_EXPORT_FLAG_NONE
	};

	D3D12_DXIL_LIBRARY_DESC raygeneration = {};
	raygeneration.DXILLibrary.BytecodeLength = sizeof(g_pRaytracing);
	raygeneration.DXILLibrary.pShaderBytecode = g_pRaytracing;
	raygeneration.NumExports = 1;
	raygeneration.pExports = &raygenExport;

	D3D12_STATE_SUBOBJECT rayGenshaders
	{
		D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
		&raygeneration
	};

	D3D12_EXPORT_DESC hitAndMissExport[2]
	{
		{
			L"Hit",
			nullptr,
			D3D12_EXPORT_FLAG_NONE
		},
		{
			L"Miss",
			nullptr,
			D3D12_EXPORT_FLAG_NONE
		}
	};

	D3D12_DXIL_LIBRARY_DESC hitAndMiss = {};
	hitAndMiss.DXILLibrary.BytecodeLength = sizeof(g_pRaytracing);
	hitAndMiss.DXILLibrary.pShaderBytecode = g_pRaytracing;
	hitAndMiss.NumExports = 2;
	hitAndMiss.pExports = hitAndMissExport;

	D3D12_STATE_SUBOBJECT hitAndMissShaders
	{
		D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
		& hitAndMiss
	};

	D3D12_HIT_GROUP_DESC hitGroup
	{
		L"HitGroup0",
		D3D12_HIT_GROUP_TYPE_TRIANGLES,
		nullptr,
		L"Hit",
		nullptr
	};
	D3D12_STATE_SUBOBJECT hitGroupObject
	{
		D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
		&hitGroup
	};

	D3D12_RAYTRACING_SHADER_CONFIG shrdConfig
	{
		sizeof(float[4]),
		sizeof(float[2])
	};
	D3D12_STATE_SUBOBJECT config
	{
		D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
		&shrdConfig
	};
	D3D12_STATE_SUBOBJECT globalRootSignature
	{
		D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
		m_globalRootSignature.GetAddressOf()
	};
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig
	{
		1
	};
	D3D12_STATE_SUBOBJECT pipeline
	{
		D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
		&pipelineConfig
	};
	std::vector<D3D12_STATE_SUBOBJECT> subObjectes;
	subObjectes.emplace_back(rayGenshaders);
	subObjectes.emplace_back(hitAndMissShaders);
	subObjectes.emplace_back(hitGroupObject);
	subObjectes.emplace_back(config);
	subObjectes.emplace_back(globalRootSignature);
	subObjectes.emplace_back(pipeline);

	D3D12_STATE_OBJECT_DESC rtStateObject
	{
		D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
		(UINT)subObjectes.size(),
		subObjectes.data()
	};
	m_device->CreateStateObject(&rtStateObject, IID_PPV_ARGS(&m_rtpso));
	ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
	ThrowIfFailed(m_rtpso.As(&stateObjectProperties));

	UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	ShaderTable parameter;
	
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(parameter)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_rgsTable)));
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(parameter)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_missTable)));
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(parameter)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_hitGroupsTable)));
	
	CD3DX12_RANGE readRange(0, 0);

	memcpy(parameter.m_mappedShaderRecords, stateObjectProperties->GetShaderIdentifier(L"RayGen"), shaderIdentifierSize);
	ThrowIfFailed(m_rgsTable->Map(0, &readRange, reinterpret_cast<void**>(&pRgsData)));
	memcpy(pRgsData, parameter.m_mappedShaderRecords, shaderIdentifierSize);

	
	memcpy(parameter.m_mappedShaderRecords, stateObjectProperties->GetShaderIdentifier(L"Miss"), shaderIdentifierSize);
	ThrowIfFailed(m_hitGroupsTable->Map(0, &readRange, reinterpret_cast<void**>(&pHitgroupsData)));
	memcpy(pHitgroupsData, parameter.m_mappedShaderRecords, shaderIdentifierSize);


	memcpy(parameter.m_mappedShaderRecords, stateObjectProperties->GetShaderIdentifier(L"HitGroup0"), shaderIdentifierSize);
	ThrowIfFailed(m_missTable->Map(0, &readRange, reinterpret_cast<void**>(&pMissData)));
	memcpy(pMissData, parameter.m_mappedShaderRecords, shaderIdentifierSize);


	ThrowIfFailed(m_dxrCommandList->Close());
	ID3D12CommandList* pCmdLists[] = { m_dxrCommandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);

	FlushCommandQueue();


	return true;
}

bool Renderer::D3D12RayTracingApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12RayTracingApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;
	return true;
}

void Renderer::D3D12RayTracingApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12RayTracingApp::InitRayTracingScene()
{
}

void Renderer::D3D12RayTracingApp::BuildAccelerationStructures()
{

}

void Renderer::D3D12RayTracingApp::Update(float& deltaTime)
{
	//D3D12App::Update(deltaTime);
}

void Renderer::D3D12RayTracingApp::UpdateGUI(float& deltaTime)
{
	//D3D12App::UpdateGUI(deltaTime);

}

void Renderer::D3D12RayTracingApp::Render(float& deltaTime)
{

	RaytracingPass(deltaTime);
	CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
}
void Renderer::D3D12RayTracingApp::RaytracingPass(float& deltaTime)
{
	auto desc = HDRRenderTargetBuffer()->GetDesc();
	UINT width = (UINT)desc.Width;
	UINT height = desc.Height;

	m_commandAllocator->Reset();
	m_dxrCommandList->Reset(m_commandAllocator.Get(), nullptr);

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), raytracingPass);


	m_dxrCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			HDRRenderTargetBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));

	m_dxrCommandList->SetComputeRootSignature(m_globalRootSignature.Get());
	m_dxrCommandList->SetPipelineState1(m_rtpso.Get());
	m_dxrCommandList->SetDescriptorHeaps(1, m_hdrUavHeap.GetAddressOf());
	m_dxrCommandList->SetComputeRootDescriptorTable(0, m_hdrUavHeap->GetGPUDescriptorHandleForHeapStart());
	m_dxrCommandList->SetComputeRootShaderResourceView(1, m_staticMeshes[0]->GetTlas());

	D3D12_DISPATCH_RAYS_DESC dispactchRays = {};
	
	dispactchRays.RayGenerationShaderRecord.StartAddress = m_rgsTable->GetGPUVirtualAddress();
	dispactchRays.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderTable);
	
	dispactchRays.HitGroupTable.StartAddress = m_hitGroupsTable->GetGPUVirtualAddress();
	dispactchRays.HitGroupTable.SizeInBytes = sizeof(ShaderTable[1]);
	dispactchRays.HitGroupTable.StrideInBytes = sizeof(ShaderTable);

	dispactchRays.MissShaderTable.StartAddress = m_missTable->GetGPUVirtualAddress();
	dispactchRays.MissShaderTable.SizeInBytes = sizeof(ShaderTable[1]);
	dispactchRays.MissShaderTable.StrideInBytes = sizeof(ShaderTable);

	dispactchRays.Width = width;
	dispactchRays.Height = height;
	dispactchRays.Depth = 1;
	m_dxrCommandList->DispatchRays(&dispactchRays);

	m_dxrCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			HDRRenderTargetBuffer(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RENDER_TARGET

		));

	ThrowIfFailed(m_dxrCommandList->Close());
	ID3D12CommandList* pCmdLists[] = { m_dxrCommandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}
void Renderer::D3D12RayTracingApp::RenderCubeMap(float& deltaTime)
{
}

void Renderer::D3D12RayTracingApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
