#pragma once
#include <memory>
#include <vector>
#include <wrl.h>
#include <d3d12.h>

#include "StaticMesh.h"
#include "MeshData.h"
namespace Animation {
	class FBX {
	public:
		FBX() {}
		~FBX() {}

		void Initialize(std::vector<PbrMeshData>& meshData, AnimationData& animationData, Microsoft::WRL::ComPtr<ID3D12Device5>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool loopAnimation = false, float animationSpeed = 1.f, 
			const Vector3& ModelTranslation = Vector3::Zero, const std::wstring& texture = L"");

		void Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap, std::map<std::wstring, unsigned int>& textureMap, UINT heapSize);

		void Update(float& deltaTime);

	private:
		void InitAnimation(AnimationData& animationData,
			double& tickPerSecond,
			double animationSpeed = 1.0,
			bool bLoop = false);

	public:
		Animation::AnimationData m_animationData;
		double m_framPerSecond = 0.f;
		double m_animationSpeed = 1.f;
		DirectX::SimpleMath::Matrix m_inverseMat = DirectX::SimpleMath::Matrix();
		DirectX::SimpleMath::Matrix m_transformFBXAnimation = DirectX::SimpleMath::Matrix();
		bool m_loopAnimation = false;
		float m_frame = 0.f;
		int m_lastFrame = 0;
		
	private:
		std::vector<std::shared_ptr<Core::StaticMesh>> m_staticMeshes;

	};

}