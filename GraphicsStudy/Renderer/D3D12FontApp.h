#pragma once

#include "D3D12App.h"

namespace Renderer {

	class D3D12FontApp :public D3D12App {
	public:
		D3D12FontApp(const int& width, const int& height);
		virtual ~D3D12FontApp() {};

		bool Initialize() override;
		bool InitDirectX() override;
		void OnResize() override;

		void Update(float& deltaTime) override;
		void Render(float& deltaTime) override;

	private:
	
	};
}