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

void Renderer::Utility::CreateTextureBuffer(const wchar_t* path, ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    using namespace DirectX;
    std::unique_ptr<uint8_t[]> ddsData;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    ThrowIfFailed(
        LoadDDSTextureFromFile(device.Get(), path,
            uploadBuffer.ReleaseAndGetAddressOf(), ddsData, subresources)
    );

}
