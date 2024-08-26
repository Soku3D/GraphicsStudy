#pragma once

#include "D3D12PassApp.h"
#include "Renderer.h"

#include "physx/PxPhysicsAPI.h"

#define PVD_HOST "127.0.0.1"
#define MAX_NUM_ACTOR_SHAPES 56

namespace Renderer {

	using namespace physx;

	class D3D12PhysxSimulationApp :public D3D12PassApp {
	public:
		D3D12PhysxSimulationApp(const int& width, const int& height);
		virtual ~D3D12PhysxSimulationApp() {
			PX_RELEASE(gScene);
			PX_RELEASE(gDispatcher);
			PX_RELEASE(gPhysics);
			if (gPvd) {
				PxPvdTransport* transport = gPvd->getTransport();
				gPvd->release();
				gPvd = NULL;
				PX_RELEASE(transport);
			}
			PX_RELEASE(gFoundation);
		}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

	protected:
		

		PxRigidDynamic* createDynamic(const PxTransform& t,
			const PxGeometry& geometry,
			const PxVec3& m_velocity = PxVec3(0));

		void CreateStack(const PxTransform& t, PxU32 size, PxReal halfExtent);

		void InitPhysics(bool interactive);

        void InitScene() override;

		PxDefaultAllocator gAllocator;
		PxDefaultErrorCallback gErrorCallback;
		PxFoundation* gFoundation = NULL;
		PxPhysics* gPhysics = NULL;
		PxDefaultCpuDispatcher* gDispatcher = NULL;
		PxScene* gScene = NULL;
		PxMaterial* gMaterial = NULL;
		PxPvd* gPvd = NULL;
		PxReal stackZ = 10.0f;

		const wchar_t* simulationRenderPassEvent = L"Simulation Render Pass ";
		const wchar_t* simulationPassEvent = L"Simulation Pass ";
	};
}