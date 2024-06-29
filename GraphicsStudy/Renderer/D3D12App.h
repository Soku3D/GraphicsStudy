#pragma once

#include "SimpleApp.h"

namespace Renderer {
	class D3D12App :public SimpleApp {
	public:
		D3D12App(const int& width, const int& height);
		virtual ~D3D12App();

		virtual bool Initialize();
		virtual bool InitDirectX() override;
		virtual void OnResize() override;
		
		virtual void Update(const double& deltaTime) override;
		virtual void Render(const double& deltaTime) override;


	};
}