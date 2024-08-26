#pragma once

#include "Constants.h"
#include "Utility.h"
#include "GeometryGenerator.h"

namespace Core {
	class StaticMesh {
	public:

		StaticMesh();
		~StaticMesh() {
			m_objectConstantData = nullptr;
			m_objectConstantBuffer.Reset();
			m_pCbvDataBegin = nullptr;
			m_vertexUploadBuffer.Reset();
			m_vertexUpload.Reset();
			m_vertexGpu.Reset();
			m_indexUpload.Reset();
			m_indexGpu.Reset();
		};
		std::vector<Animation::AnimationClip::Key> m_keys;
		std::string m_name;
		double m_secondPerFrames = 0.f;
		double m_animationSpeed = 1.f;
		DirectX::SimpleMath::Matrix m_inverseMat = DirectX::SimpleMath::Matrix();
		DirectX::SimpleMath::Matrix m_transformFBXAnimation = DirectX::SimpleMath::Matrix();
		bool m_loopAnimation = false;
		float frame = 0.f;

		template <typename Vertex>
		void Initialize(MeshData<Vertex>& meshData,
			Microsoft::WRL::ComPtr<ID3D12Device5>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
			const DirectX::SimpleMath::Vector3& modelPosition = DirectX::SimpleMath::Vector3::Zero,
			Material& material = Material(),
			bool bUseAoMap = false,
			bool bUseHeightMap = false,
			bool bUseMetalnessMap = false,
			bool bUseNormalMap = false,
			bool bUseRoughnessMap = false,
			bool bUseTesslation = false)
		{
			m_name = meshData.m_name;

			Renderer::Utility::CreateBuffer(meshData.m_vertices, m_vertexUpload, m_vertexGpu, device, commandList);
			Renderer::Utility::CreateBuffer(meshData.m_indices, m_indexUpload, m_indexGpu, device, commandList);

			m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
			m_vertexBufferView.SizeInBytes = (UINT)(meshData.m_vertices.size() * sizeof(Vertex));
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);

			m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
			m_indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * meshData.m_indices.size());
			m_objectConstantData = new ObjectConstantData();

			m_objectConstantData->Material = material;
			m_objectConstantData->bUseAoMap = bUseAoMap;
			m_objectConstantData->bUseHeightMap = bUseHeightMap;
			m_objectConstantData->bUseMetalnessMap = bUseMetalnessMap;
			m_objectConstantData->bUseNormalMap = bUseNormalMap;
			m_objectConstantData->bUseRoughnessMap = bUseRoughnessMap;
			m_objectConstantData->bUseTesslation = bUseTesslation;

			m_objectConstantData->Model = DirectX::XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
			m_transformFBXAnimation = m_objectConstantData->Model;
			m_objectConstantData->invTranspose = m_objectConstantData->Model.Invert();
			m_objectConstantData->Model = m_objectConstantData->Model.Transpose();

			std::vector<ObjectConstantData> constantData = { *m_objectConstantData };
			Renderer::Utility::CreateUploadBuffer(constantData, m_objectConstantBuffer, device);
			indexCount = (UINT)meshData.m_indices.size();

			CD3DX12_RANGE range(0, 0);
			ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
			memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
			m_texturePath = meshData.GetTexturePath();
		}
		template <typename Vertex>
		void BuildAccelerationStructures(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>& commandList)
		{
			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
			geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			geometryDesc.Triangles.IndexBuffer = m_indexGpu->GetGPUVirtualAddress();
			geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexGpu->GetDesc().Width) / sizeof(uint32_t);
			geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			geometryDesc.Triangles.Transform3x4 = 0;
			geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexGpu->GetDesc().Width) / sizeof(Vertex);
			geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexGpu->GetGPUVirtualAddress();
			geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
			topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			topLevelInputs.Flags = buildFlags;
			topLevelInputs.NumDescs = 1;
			topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
			device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
			bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			bottomLevelInputs.pGeometryDescs = &geometryDesc;
			device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

			if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes < 0)
			{
				std::cout << "Failed GetRaytracingAccelerationStructurePrebuildInfo(BLAS)\n";
			}

			UINT64 buffersize = max(bottomLevelPrebuildInfo.ScratchDataSizeInBytes, topLevelPrebuildInfo.ScratchDataSizeInBytes);
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(buffersize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&m_scratchResource)));

			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				nullptr,
				IID_PPV_ARGS(&m_blas)));

			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				nullptr,
				IID_PPV_ARGS(&m_tlas)));


			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
			instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = m_blas->GetGPUVirtualAddress();
			UINT64 datasize = sizeof(instanceDesc);

			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(datasize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_instanceDescs)));

			void* pMappedData;
			m_instanceDescs->Map(0, nullptr, &pMappedData);
			memcpy(pMappedData, &instanceDesc, datasize);
			m_instanceDescs->Unmap(0, nullptr);

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};

			bottomLevelBuildDesc.Inputs = bottomLevelInputs;
			bottomLevelBuildDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
			bottomLevelBuildDesc.DestAccelerationStructureData = m_blas->GetGPUVirtualAddress();

			commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};

			topLevelInputs.InstanceDescs = m_instanceDescs->GetGPUVirtualAddress();
			topLevelBuildDesc.Inputs = topLevelInputs;
			topLevelBuildDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
			topLevelBuildDesc.DestAccelerationStructureData = m_tlas->GetGPUVirtualAddress();

			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_blas.Get()));

			commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_tlas.Get()));
			std::wstringstream tlassName;
			std::wstring name(m_name.begin(), m_name.end());
			tlassName << L"TLAS - " << name;
			std::wstringstream blassName;
			blassName << L"BLAS - " << name;

			m_tlas->SetName(tlassName.str().c_str());
			m_blas->SetName(blassName.str().c_str());
			//Renderer::Utility::CreateDescriptorHeap(device, m_blasSrvHeap, Renderer::DescriptorType::SRV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, L"blas Srv Heap");
			//Renderer::Utility::CreateDescriptorHeap(device, m_tlasSrvHeap, Renderer::DescriptorType::SRV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, L"tlas Srv Heap");
			//device->CreateShaderResourceView(m_blas.Get(), nullptr, m_blasSrvHeap->GetCPUDescriptorHandleForHeapStart());
			//device->CreateShaderResourceView(m_tlas.Get(), nullptr, m_tlasSrvHeap->GetCPUDescriptorHandleForHeapStart());
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetBlas() { return m_blas->GetGPUVirtualAddress(); }
		D3D12_GPU_VIRTUAL_ADDRESS GetTlas() { return m_tlas->GetGPUVirtualAddress(); }
		void Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat = true);
		void RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat);
		void UpdateAnimation(const float& deltaTime, Animation::AnimationData& animationData);
		void Update(const float& deltaTime);

		//void UpdateDomain(const float& deltaTime, float gui_edge0, float gui_edge1, float gui_edge2, float gui_edge3, float gui_inside0, float gui_inside1);

		std::wstring GetTexturePath() const { return m_texturePath; }
		void SetTexturePath(std::wstring path) { m_texturePath = path; }
		void UpdateWorldRow(const DirectX::SimpleMath::Matrix& worldRow);
		void UpdateMaterial(const Material& material);
		Material& GetMaterial() const;
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_objectConstantBuffer;
		ObjectConstantData* m_objectConstantData;
		UINT8* m_pCbvDataBegin;
		float m_delTeta = 1.f / (3.141592f * 2.f);
		float m_currTheta = 0.f;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUploadBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexGpu;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexGpu;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_scratchResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_blas;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_tlas;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_instanceDescs;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_blasSrvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_tlasSrvHeap;

		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		UINT indexCount = 0;
		std::wstring m_texturePath = L"";
	};
}