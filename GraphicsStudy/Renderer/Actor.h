#pragma once
#include "Object.h"
#include "SceneComponent.h"

namespace Core {
	class Actor : public Object {
	public:
		Actor();
		virtual ~Actor();
		
	public:
		virtual void Tick(float deltaTime) = 0;
		virtual void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12CommandList>& commandList);
	protected:
		std::shared_ptr<class SceneComponent> m_rootComponent;
	};
}
