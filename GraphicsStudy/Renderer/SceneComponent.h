#pragma once
#include "ActorComponent.h"

namespace Core {
	class SceneComponent : public ActorComponent {
	public:
		SceneComponent() {};
		virtual ~SceneComponent() {};

		void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12CommandList>& commandList) override;
	};
}
