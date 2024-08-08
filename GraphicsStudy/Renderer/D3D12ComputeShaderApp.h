#pragma once

#include "D3D12App.h"

namespace Renderer {
	
	class D3D12ComputeShaderApp :public D3D12App {
	public:
		D3D12ComputeShaderApp(const int& width, const int& height);
		virtual ~D3D12ComputeShaderApp() {}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;


	};
}