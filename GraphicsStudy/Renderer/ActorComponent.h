#pragma once

#include "Object.h"

namespace Core {

	class ActorComponent : public Object{

	public:
		ActorComponent() {};
		virtual ~ActorComponent() {};

	public:
		virtual void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12CommandList>& commandList) = 0;
	};
}