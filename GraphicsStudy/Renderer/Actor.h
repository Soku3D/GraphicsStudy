#pragma once
//#include "SceneComponent.h"

namespace Core {

	class Actor {
	public:
		Actor();
		virtual ~Actor() {}
		
	public:
		//virtual void Tick(float deltaTime) = 0;
		//virtual void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12CommandList>& commandList);
		virtual void RotateDirection() = 0;
		virtual void MoveUp(float deltaTime) = 0;
		virtual void MoveDown(float deltaTime) = 0;
		virtual void MoveRight(float deltaTime) = 0;
		virtual void MoveLeft(float deltaTime) = 0;
		virtual void MoveForward(float deltaTime) = 0;
		virtual void MoveBackward(float deltaTime) = 0;
	//protected:
		//std::shared_ptr<class SceneComponent> m_rootComponent;
	};
}
