#pragma once
#include "PrimitiveComponent.h"

namespace Core {
	class BoxComponent : public PrimitiveComponent{
	public:
		BoxComponent();
		virtual ~BoxComponent();

		void Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12CommandList>& commandList) override;
	};
}