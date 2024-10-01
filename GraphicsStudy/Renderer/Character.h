#pragma once
#include "Actor.h"

namespace Core {

	class Character : public Actor {
	public:
		Character();
		virtual ~Character();

	public:
		void Update(float deltaTime) override;
		void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, int index = 0);
		void RenderBoundingBox(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		void RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int index);
		void RotateDirection() override;
		void MoveUp(float deltaTime) override;
		void MoveDown(float deltaTime) override;
		void MoveRight(float deltaTime) override;
		void MoveLeft(float deltaTime) override;
		void MoveForward(float deltaTime) override;
		void MoveBackward(float deltaTime) override;
		
		size_t GetMeshCount() const;
		void SetRotation(int deltaX, int deltaY);
		void SetCameraAspectRatio(float ratio);

		DirectX::SimpleMath::Matrix GetViewMatrix();
		DirectX::SimpleMath::Matrix GetProjMatrix();
		DirectX::SimpleMath::Vector3 GetViewDirection();
		DirectX::SimpleMath::Vector3 GetForwardDirection();
		DirectX::SimpleMath::Vector3 GetUpDirection();
		DirectX::SimpleMath::Vector3 GetCameraPosition();

		DirectX::SimpleMath::Vector3 GetPosition();

	protected:
		class CameraComponent* mCamera;
	};
}