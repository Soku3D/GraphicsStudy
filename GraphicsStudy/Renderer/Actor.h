#pragma once
//#include "SceneComponent.h"
#include <wrl.h>
#include <d3d12.h>
#include <directxtk/SimpleMath.h>
#include <string>
#include "RaytracingHlslCompat.h"

namespace Core {

	class Actor {
	public:
		Actor();
		virtual ~Actor() {
		}
		
	public:
		virtual void Update(float deltaTime);
		virtual void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, int index = 0);
		virtual void RenderBoundingBox(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		void RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int index);
		virtual void RotateDirection() {};
		virtual void MoveUp(float deltaTime) {};
		virtual void MoveDown(float deltaTime) {};
		virtual void MoveRight(float deltaTime) {};
		virtual void MoveLeft(float deltaTime) {};
		virtual void MoveForward(float deltaTime) {};
		virtual void MoveBackward(float deltaTime){};

		void SetStaticMeshComponent(class StaticMesh* staticMesh);
		std::wstring GetTexturePath(int index = 0) const;
		DirectX::SimpleMath::Vector3 GetPosition() const { return mPosition; }
		void SetPosition(const DirectX::SimpleMath::Vector3& position);
		void SetForwardDirection(const DirectX::SimpleMath::Vector3& direction);
		void SetVelocity(const float& velocity);
		
		float GetYTheta() const { return m_yTheta; };
		DirectX::SimpleMath::Matrix GetTransformMatrix();
		D3D12_GPU_VIRTUAL_ADDRESS GetBlas(int index = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE GetIndexCpuHandle(int index = 0);
		PrimitiveConstantBuffer GetprimitiveConstantData();

	protected:

		DirectX::SimpleMath::Vector3 mPosition;
		DirectX::SimpleMath::Vector3 mViewDirection;
		DirectX::SimpleMath::Vector3 mForwardDirection;
		DirectX::SimpleMath::Vector3 mStandardDirection;
		DirectX::SimpleMath::Vector3 mRightDirection;
		DirectX::SimpleMath::Vector3 mUpDirection;
		float mVelocity = 1.f;
		float m_delTheta;
		float m_delSine;
		float m_delCosine;

		DirectX::SimpleMath::Quaternion m_quaternion;
		float m_xTheta = 0.0;
		float m_yTheta = 0.0;

	protected:
		class Core::StaticMesh* mStaticMesh;
	};
}
