#include "Utility.h"
#include <comdef.h>
#include "directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"
DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}
std::wstring DxException::ToString()const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}



void Renderer::Utility::CreateTextureBuffer(const wchar_t* path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12DescriptorHeap>& heap,
    ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandQueue>& commandQueue)
{
    using namespace DirectX;

    ResourceUploadBatch resourceUpload(device.Get());

    resourceUpload.Begin();

    ThrowIfFailed(
       CreateDDSTextureFromFile(device.Get(), resourceUpload, path,
            texture.ReleaseAndGetAddressOf())
    );

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
    device->CreateShaderResourceView(texture.Get(), &srvDesc, heap->GetCPUDescriptorHandleForHeapStart());

    auto uploadResourcesFinished = resourceUpload.End(
        commandQueue.Get());

    uploadResourcesFinished.wait();
}
