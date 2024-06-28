#pragma once

#include <string>
#include <windows.h>
#include <fstream>
#include <vector>
#include <iostream>

#include "Timer.h"
#include <cassert>

#include "dxgi.h"
#include "Vertex.h"
#include "d3d11.h"
#include "d3d12.h"
#include "wrl.h"
#include "d3dcompiler.h"

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif
class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};
namespace Renderer {


	class Utility {
	public:
		template<typename Vertex>
		static void CreateVertexBuffer(std::vector<Vertex>& vertices,
			Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer,
			Microsoft::WRL::ComPtr<ID3D11Device>& device) {
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = sizeof(Vertex) * vertices.size();
			buffDesc.Usage = D3D11_USAGE_IMMUTABLE;
			buffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			buffDesc.CPUAccessFlags = false;
			buffDesc.MiscFlags = 0;
			buffDesc.StructureByteStride = sizeof(Vertex);

			D3D11_SUBRESOURCE_DATA subData;
			ZeroMemory(&subData, sizeof(subData));
			subData.pSysMem = vertices.data();

			ThrowIfFailed(device->CreateBuffer(&buffDesc, &subData, buffer.GetAddressOf()));
		}
		template<typename Index>
		static void CreateIndexBuffer(std::vector<Index>& indices,
			Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer,
			Microsoft::WRL::ComPtr<ID3D11Device>& device) {
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = sizeof(Index) * indices.size();
			buffDesc.Usage = D3D11_USAGE_IMMUTABLE;
			buffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
			buffDesc.CPUAccessFlags = false;
			buffDesc.MiscFlags = 0;
			buffDesc.StructureByteStride = sizeof(Index);

			D3D11_SUBRESOURCE_DATA subData;
			ZeroMemory(&subData, sizeof(subData));
			subData.pSysMem = indices.data();

			ThrowIfFailed(device->CreateBuffer(&buffDesc, &subData, buffer.GetAddressOf()));
		}
		static void CreateVertexShaderAndInputLayout(
			const std::wstring& shaderPath,
			Microsoft::WRL::ComPtr<ID3D11VertexShader>& vertexShader,
			std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
			Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputlayout,
			Microsoft::WRL::ComPtr<ID3D11Device>& device) {

			Microsoft::WRL::ComPtr <ID3DBlob> shader;
			Microsoft::WRL::ComPtr <ID3DBlob> error;

			ThrowIfFailed(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "main",
				"vs_5_0", 0, 0, shader.GetAddressOf(), error.GetAddressOf()));

			ThrowIfFailed(device->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr,
				vertexShader.GetAddressOf()));

			ThrowIfFailed(device->CreateInputLayout(elements.data(), elements.size(), shader->GetBufferPointer(), shader->GetBufferSize(),
				inputlayout.GetAddressOf()));
		}
		static void CreatePixelShader(
			const std::wstring& shaderPath,
			Microsoft::WRL::ComPtr<ID3D11PixelShader>& pixelShader,
			Microsoft::WRL::ComPtr<ID3D11Device>& device) {

			Microsoft::WRL::ComPtr <ID3DBlob> shader;
			Microsoft::WRL::ComPtr <ID3DBlob> error;

			ThrowIfFailed(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "main",
				nullptr, 0, 0, shader.GetAddressOf(), error.GetAddressOf()));

			ThrowIfFailed(device->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr,
				pixelShader.GetAddressOf()));
		}
	};
}