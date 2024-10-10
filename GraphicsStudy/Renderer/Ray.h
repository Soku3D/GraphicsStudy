#pragma once

#include "Constants.h"
#include "Utility.h"
#include "GeometryGenerator.h"
#include "RaytracingHlslCompat.h"

namespace Core {
	class Ray {
	public:

		Ray();
		virtual ~Ray();
		void Update(const float& deltaTime);
		void Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);


		void Initialize(std::vector<DirectX::SimpleMath::Vector3>& vertices,
			Microsoft::WRL::ComPtr<ID3D12Device5>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);


	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mVertexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> mVertexGpu;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
		int boneCount = 0;
	};
}