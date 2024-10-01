#pragma once

#include "D3D12PassApp.h"
#include "Renderer.h"
#include "Buffer.h"
#include "Constants.h"

namespace Renderer {

	class D3D12DLSSApp :public D3D12PassApp {
	public:
		D3D12DLSSApp(const int& width, const int& height);
		virtual ~D3D12DLSSApp() {}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void InitScene() override;
		void OnResize() override;
		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderMotionVectorPass(float& deltaTime);

		void RenderGUI(float& deltaTime) override;

	private:
		bool guiUseDLAA = true;
	

	};

}