#pragma once

#include "Utility.h"

class ShaderRecord
{
public:
    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
    {
    }

    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
        localRootArguments(pLocalRootArguments, localRootArgumentsSize)
    {
    }
public:
    struct PointerWithSize {
        void* ptr;
        UINT size;

        PointerWithSize() : ptr(nullptr), size(0) {}
        PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
    };
    PointerWithSize shaderIdentifier;
    PointerWithSize localRootArguments;
};
class ShaderTable {

public:
	ShaderTable();
    ShaderTable(Microsoft::WRL::ComPtr<ID3D12Device5>& device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr) 
    {
        UINT64 bufferSize = numShaderRecords * shaderRecordSize;
        Renderer::Utility::CreateBuffer(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, bufferSize,
            D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_table);
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(m_table->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedShaderRecoreds)));
    }

    void push_back(const ShaderRecord& shaderRecord) {
       
        memcpy(m_mappedShaderRecoreds, shaderRecord.shaderIdentifier.ptr, shaderRecord.shaderIdentifier.size);
        m_mappedShaderRecoreds += shaderRecord.shaderIdentifier.size;
        if (shaderRecord.localRootArguments.size > 0) {
            
            memcpy(m_mappedShaderRecoreds, shaderRecord.localRootArguments.ptr, shaderRecord.localRootArguments.size);
            m_mappedShaderRecoreds += shaderRecord.localRootArguments.size;
        }
    }
    ComPtr<ID3D12Resource> GetResource() { return m_table; }
    uint8_t* m_mappedShaderRecoreds;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_table;
};
