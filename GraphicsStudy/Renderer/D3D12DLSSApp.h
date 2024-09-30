#pragma once

#include "D3D12PassApp.h"
#include "Renderer.h"
#include "Buffer.h"
#include "Constants.h"

#include <sl.h>
#include <sl_dlss.h>
#include <sl_consts.h>

namespace Renderer {

	class D3D12DLSSApp :public D3D12PassApp {
	public:
		D3D12DLSSApp(const int& width, const int& height);
		virtual ~D3D12DLSSApp() {
		}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void InitScene() override;
		void OnResize() override;

		bool InitializeDLSS();
		void ApplyAntiAliasing();
		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderMotionVectorPass(float& deltaTime);

		void RenderGUI(float& deltaTime) override;

	private:
		sl::FrameToken* mCurrentFrame;
		sl::ViewportHandle mViewport = { 0 };
		Core::ConstantBuffer< GlobalConstantDataDLSS> mDlssConstantBuffer;

		DirectX::SimpleMath::Matrix prevView;
		DirectX::SimpleMath::Matrix prevProj;

		const wchar_t* renderMotionVectorPassEvent = L"Render MotionVector Pass ";

	};

}