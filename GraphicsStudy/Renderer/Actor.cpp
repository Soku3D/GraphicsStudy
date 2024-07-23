#include "Actor.h"

void Core::Actor::Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12CommandList>& commandList)
{
	if(m_rootComponent!=nullptr)
		m_rootComponent->Render(deltaTime, commandList);
}
