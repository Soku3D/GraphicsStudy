#include "SkeletonMesh.h"
#include "Utility.h"
#include "directxtk\SimpleMath.h"
Core::SkeletonMesh::SkeletonMesh()
{
}

Core::SkeletonMesh::~SkeletonMesh()
{
}

void Core::SkeletonMesh::Update(const float& deltaTime)
{
}

void Core::SkeletonMesh::Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    commandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    commandList->DrawInstanced(boneCount, 1, 0, 0);
}

void Core::SkeletonMesh::Initialize(std::vector<DirectX::SimpleMath::Vector3>& vertices, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{    
    size_t bufferSize = vertices.size() * sizeof(DirectX::SimpleMath::Vector3);
    // Vertex Buffer 리소스 생성
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mVertexGpu)
    ));

    // 버퍼에 데이터 복사
    void* mappedData;
    mVertexGpu->Map(0, nullptr, &mappedData);
    memcpy(mappedData, vertices.data(), bufferSize);
    mVertexGpu->Unmap(0, nullptr);

    boneCount = vertices.size();

    mVertexBufferView.BufferLocation = mVertexGpu->GetGPUVirtualAddress();
    mVertexBufferView.SizeInBytes = vertices.size() * sizeof(DirectX::SimpleMath::Vector3);
    mVertexBufferView.StrideInBytes = sizeof(DirectX::SimpleMath::Vector3);

}
