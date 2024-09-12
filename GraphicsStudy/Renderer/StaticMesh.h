#pragma once

#include "Constants.h"
#include "Utility.h"
#include "GeometryGenerator.h"
#include "RaytracingHlslCompat.h"

namespace Core {
	class StaticMesh {
	public:

		StaticMesh();
		~StaticMesh() {
			m_objectConstantData = nullptr;
			m_objectConstantBuffer.Reset();
			m_pCbvDataBegin = nullptr;
		};

		std::vector<Animation::AnimationClip::Key> m_keys;
		std::string m_name;
		double m_secondPerFrames = 0.f;
		double m_animationSpeed = 1.f;
		DirectX::SimpleMath::Matrix m_inverseMat = DirectX::SimpleMath::Matrix();
		DirectX::SimpleMath::Matrix m_transformFBXAnimation = DirectX::SimpleMath::Matrix();
		bool m_loopAnimation = false;
		float frame = 0.f;
		bool bIsCubeMap = false;

		UINT meshCount = 0;

		PrimitiveConstantBuffer m_primitiveConstantData;

		D3D12_GPU_VIRTUAL_ADDRESS GetBlas(int index = 0) { return m_blas[index]->GetGPUVirtualAddress(); }
		void Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat = true, int index =  0);
		void RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int index = 0);
		void RenderBoundingBox(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		void UpdateAnimation(const float& deltaTime, Animation::AnimationData& animationData);
		void Update(const float& deltaTime);

		std::wstring GetTexturePath(int index = 0) const { return mTexturePath[index]; }
		void SetIsCubeMap(bool isCubeMap) { bIsCubeMap = isCubeMap; }
		void SetTexturePath(std::wstring path, int index = 0) { mTexturePath[index] = path; }
		void SetBoundingBoxHalfLength(const float& halfLength) { m_objectConstantData->boundingBoxHalfLength = halfLength; }
		void UpdateWorldRow(const DirectX::SimpleMath::Matrix& worldRow);
		void UpdateMaterial(const Material& material);
		Material& GetMaterial() const;
		DirectX::SimpleMath::Matrix GetTransformMatrix() { return m_objectConstantData->Model; }

		D3D12_GPU_DESCRIPTOR_HANDLE GetIndexGpuHandle(int index = 0) { return mIndicesSrvHeap[index]->GetGPUDescriptorHandleForHeapStart(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetIndexCpuHandle(int index = 0) { return mIndicesSrvHeap[index]->GetCPUDescriptorHandleForHeapStart(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_objectConstantBuffer;
		ObjectConstantData* m_objectConstantData;
		UINT8* m_pCbvDataBegin;
		float m_delTeta = 1.f / (3.141592f * 2.f);
		float m_currTheta = 0.f;
		
		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> mScratchResource;
		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> m_blas;
		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> m_instanceDescs;
		std::vector < Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_blasSrvHeap;

		std::vector < Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> mIndicesSrvHeap;

	private:
		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> mVertexUpload;
		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> mVertexGpu;
		std::vector < D3D12_VERTEX_BUFFER_VIEW> mVertexBufferView;

		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> mIndexUpload;
		std::vector < Microsoft::WRL::ComPtr<ID3D12Resource>> mIndexGpu;

	
		std::vector <D3D12_INDEX_BUFFER_VIEW> mIndexBufferView;
		std::vector <UINT> mIndexCount;
		std::vector <UINT> mVertexCount;
		std::vector<std::wstring> mTexturePath;

	public:
		template <typename Vertex, typename Index>
		void Initialize(MeshData<Vertex, Index> & meshData,
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
			std::vector< MeshData<Vertex, Index>> meshes = { meshData };
			Initialize(meshes, device, commandList, modelPosition, material,
				bUseAoMap,
				bUseHeightMap,
				bUseMetalnessMap,
				bUseNormalMap,
				bUseRoughnessMap,
				bUseTesslation);
		}

		template <typename Vertex, typename Index>
		void Initialize(std::vector<MeshData<Vertex, Index>>& meshData,
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
			m_name = meshData[0].m_name;

			mVertexUpload.resize(meshData.size());
			mVertexGpu.resize(meshData.size());
			mVertexBufferView.resize(meshData.size());
			mIndexUpload.resize(meshData.size());
			mIndexGpu.resize(meshData.size());
			mIndexBufferView.resize(meshData.size());
			mIndexCount.resize(meshData.size());
			mVertexCount.resize(meshData.size());
			mTexturePath.resize(meshData.size());
			mIndicesSrvHeap.resize(meshData.size());
			meshCount = (UINT)meshData.size();

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

			m_primitiveConstantData.invTranspose = m_objectConstantData->invTranspose;

			std::vector<ObjectConstantData> constantData = { *m_objectConstantData };
			Renderer::Utility::CreateUploadBuffer(constantData, m_objectConstantBuffer, device);

			CD3DX12_RANGE range(0, 0);
			ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
			memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));

			for (size_t i = 0; i < meshData.size(); i++)
			{
				Renderer::Utility::CreateBuffer(meshData[i].m_vertices, mVertexUpload[i], mVertexGpu[i], device, commandList);
				Renderer::Utility::CreateBuffer(meshData[i].m_indices, mIndexUpload[i], mIndexGpu[i], device, commandList);

				mVertexBufferView[i].BufferLocation = mVertexGpu[i]->GetGPUVirtualAddress();
				mVertexBufferView[i].SizeInBytes = (UINT)(meshData[i].m_vertices.size() * sizeof(Vertex));
				mVertexBufferView[i].StrideInBytes = sizeof(Vertex);

				mIndexBufferView[i].BufferLocation = mIndexGpu[i]->GetGPUVirtualAddress();

				if (sizeof(Index) == 2) {
					mIndexBufferView[i].Format = DXGI_FORMAT_R16_UINT;
				}
				else {
					mIndexBufferView[i].Format = DXGI_FORMAT_R32_UINT;
				}
				mIndexBufferView[i].SizeInBytes = UINT(sizeof(Index) * meshData[i].m_indices.size());
				mIndexCount[i] = (UINT)meshData[i].m_indices.size();
				mVertexCount[i] = (UINT)meshData[i].m_vertices.size();
				mTexturePath[i] = meshData[i].GetTexturePath();

				Renderer::Utility::CreateDescriptorHeap(device, mIndicesSrvHeap[i], Renderer::DescriptorType::SRV, 2);

				D3D12_SHADER_RESOURCE_VIEW_DESC SRVDescVertex = {};
				SRVDescVertex.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				SRVDescVertex.Buffer.NumElements = (UINT)meshData[i].m_vertices.size();
				SRVDescVertex.Buffer.FirstElement = 0;
				SRVDescVertex.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				SRVDescVertex.Format = DXGI_FORMAT_UNKNOWN;
				SRVDescVertex.Buffer.StructureByteStride = sizeof(Vertex);
				SRVDescVertex.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

				D3D12_SHADER_RESOURCE_VIEW_DESC SRVDescIndex = {};
				SRVDescIndex.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				SRVDescIndex.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				SRVDescIndex.Buffer.NumElements = (UINT)(meshData[i].m_indices.size());
				SRVDescIndex.Format = DXGI_FORMAT_R32_TYPELESS;
				SRVDescIndex.Buffer.StructureByteStride = 0;
				SRVDescIndex.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

				CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mIndicesSrvHeap[i]->GetCPUDescriptorHandleForHeapStart());
				UINT srvIncreaseSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				device->CreateShaderResourceView(mIndexGpu[i].Get(), &SRVDescIndex, handle);
				handle.Offset(1, srvIncreaseSize);
				device->CreateShaderResourceView(mVertexGpu[i].Get(), &SRVDescVertex, handle);
			}
		}

		template <typename Vertex, typename Index>
		void BuildAccelerationStructures(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>& commandList)
		{
			mScratchResource.resize(meshCount);
			m_blas.resize(meshCount);
			m_instanceDescs.resize(meshCount);

			for (UINT i = 0; i < meshCount; i++)
			{
				D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
				geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				geometryDesc.Triangles.IndexBuffer = mIndexGpu[i]->GetGPUVirtualAddress();
				geometryDesc.Triangles.IndexCount = static_cast<UINT>(mIndexGpu[i]->GetDesc().Width) / sizeof(Index);
				geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
				geometryDesc.Triangles.Transform3x4 = 0;
				geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				geometryDesc.Triangles.VertexCount = static_cast<UINT>(mVertexGpu[i]->GetDesc().Width) / sizeof(Vertex);
				geometryDesc.Triangles.VertexBuffer.StartAddress = mVertexGpu[i]->GetGPUVirtualAddress();
				geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
				geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {};
				bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
				bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
				bottomLevelInputs.NumDescs = 1;
				bottomLevelInputs.pGeometryDescs = &geometryDesc;
				device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

				if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes < 0)
				{
					std::cout << "Failed GetRaytracingAccelerationStructurePrebuildInfo(BLAS)\n";
					return;
				}

				UINT64 buffersize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;

				Renderer::Utility::CreateBuffer(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE,
					buffersize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_COMMON,
					mScratchResource[i]);

				Renderer::Utility::CreateBuffer(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE,
					bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, m_blas[i]);

				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};

				bottomLevelBuildDesc.Inputs = bottomLevelInputs;
				bottomLevelBuildDesc.ScratchAccelerationStructureData = mScratchResource[i]->GetGPUVirtualAddress();
				bottomLevelBuildDesc.DestAccelerationStructureData = m_blas[i]->GetGPUVirtualAddress();

				commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_blas[i].Get()));
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(mScratchResource[i].Get()));

				std::wstring name(m_name.begin(), m_name.end());
				std::wstringstream blassName;
				blassName << L"BLAS - " << name;

				//m_tlas->SetName(tlassName.str().c_str());
				m_blas[i]->SetName(blassName.str().c_str());
			}
		}
	};
}