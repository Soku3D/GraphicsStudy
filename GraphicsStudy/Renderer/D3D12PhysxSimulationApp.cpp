#include "D3D12PhysxSimulationApp.h"

#include "GeometryGenerator.h"


static physx::PxFilterFlags contactReportFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	using namespace physx;

	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	//PX_UNUSED(filterData0);
	//PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	// all initial and persisting reports for everything, with per-point data
	//if(filterData0.word0)
	pairFlags =
		PxPairFlag::eSOLVE_CONTACT |
		PxPairFlag::eDETECT_DISCRETE_CONTACT;/* |
		PxPairFlag::eNOTIFY_TOUCH_FOUND |
		PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
		PxPairFlag::eNOTIFY_CONTACT_POINTS*/

	return PxFilterFlag::eDEFAULT;
}

Renderer::D3D12PhysxSimulationApp::D3D12PhysxSimulationApp(const int& width, const int& height)
	:D3D12PassApp(width, height)
{
	bUseGUI = true;
	bRenderCubeMap = true;
	bRenderMeshes = true;
	bRenderFbx = false;
	bRenderNormal = false;

	m_camera->SetPositionAndDirection(DirectX::SimpleMath::Vector3(7.27f, 7.35f, -3.66f),
		DirectX::SimpleMath::Vector3(-0.55f, -0.62f, 0.55f));

	m_appName = "PhysxSimulationApp";
}

bool Renderer::D3D12PhysxSimulationApp::Initialize()
{
	if (!D3D12PassApp::Initialize())
		return false;

	//myCallback = new MySimulationEventCallback();
	m_camera->SetSpeed(3.f);
	return true;
}

bool Renderer::D3D12PhysxSimulationApp::InitGUI()
{
	if (!D3D12PassApp::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12PhysxSimulationApp::InitDirectX()
{
	if (!D3D12PassApp::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12PhysxSimulationApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12PhysxSimulationApp::Update(float& deltaTime)
{
	if (fire) {
		auto cameraPos = m_camera->GetPosition();
		auto cameraForwardDir = m_camera->GetForwardDirection();
		CreateDynamicSphere(cameraPos, cameraForwardDir, 100);

		fire = false;
		//std::cout << "Fire!";
		PlaySoundEffect("Shoting", m_camera->GetPosition() + m_camera->GetForwardDirection(), 0.3f);
	}

	gScene->simulate(min(deltaTime, 1 / 30.f));
	gScene->fetchResults(true);

	// gScene->getActors()
	// PxGeometryType::eBOX: , case PxGeometryType::eSPHERE:

	for (size_t i = 0; i < gContactPositions.size(); ++i) 
	{
		PlaySoundEffect("HitGround", DirectX::SimpleMath::Vector3(gContactPositions[i].x, gContactPositions[i].y, gContactPositions[i].z), 2.f);
	}
	gContactPositions.clear();

	PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
		PxActorTypeFlag::eRIGID_STATIC);

	std::vector<PxRigidActor*> actors(nbActors);
	gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC |
		PxActorTypeFlag::eRIGID_STATIC,
		reinterpret_cast<PxActor**>(&actors[0]), nbActors);

	PxShape* shapes[MAX_NUM_ACTOR_SHAPES];

	for (PxU32 i = 0; i < nbActors; i++) {

		const PxU32 nbShapes = actors[i]->getNbShapes();
		PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
		actors[i]->getShapes(shapes, nbShapes);

		for (PxU32 j = 0; j < nbShapes; j++) {
			const PxMat44 shapePose(
				PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));

			if (actors[i]->is<PxRigidDynamic>()) {

				bool sleeping = actors[i]->is<PxRigidDynamic>() &&
					actors[i]->is<PxRigidDynamic>()->isSleeping();
				if (sleeping) {
					m_staticMeshes[i]->UpdateMaterial(Material(1.f, 0.2f, 1.f, 0.3f));
				}
				else
				{
					m_staticMeshes[i]->UpdateMaterial(Material(0.7f, 0.3f, 0.5f, 0.3f));
				}
				m_staticMeshes[i]->UpdateWorldRow(DirectX::SimpleMath::Matrix(shapePose.front()) *
					DirectX::SimpleMath::Matrix::CreateScale(1.00f));
			}
		}
	}
	D3D12PassApp::Update(deltaTime);
}

void Renderer::D3D12PhysxSimulationApp::UpdateGUI(float& deltaTime)
{
	D3D12PassApp::UpdateGUI(deltaTime);
}

void Renderer::D3D12PhysxSimulationApp::Render(float& deltaTime)
{
	D3D12PassApp::Render(deltaTime);
}

void Renderer::D3D12PhysxSimulationApp::RenderGUI(float& deltaTime)
{
	D3D12PassApp::RenderGUI(deltaTime);
}

void Renderer::D3D12PhysxSimulationApp::CreateStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PbrMeshData box = GeometryGenerator::PbrBox(halfExtent);
	PxFilterData filterData;
	filterData.word0 = 1;
	PxShape* shape =
		gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);


	shape->setSimulationFilterData(filterData);
	static int index = 0;

	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			std::stringstream ss;
			ss << "box" << index++;

			std::shared_ptr<Core::StaticMesh> boxMesh = std::make_shared<Core::StaticMesh>();
			box.m_name = ss.str();
			boxMesh->Initialize(box, m_device, m_commandList, DirectX::SimpleMath::Vector3::Zero,
				Material(0.7f, 0.3f, 0.5f, 0.3f), false, false, false, false, false, false);
			boxMesh->SetBoundingBoxHalfLength(halfExtent);
			m_staticMeshes.push_back(boxMesh);

			PxVec3 di = PxVec3(4.f / 3.f, 2.f, 0.f) * halfExtent * PxReal(i);
			PxVec3 dj = PxVec3(8.f / 3.f, 0.f, 0.f) * halfExtent * PxReal(j);


			PxTransform localTm(di + dj + PxVec3(0, halfExtent, 0.f));
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->setName(boxMesh->m_name.c_str());
			body->attachShape(*shape);
			
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

			gScene->addActor(*body);
		}
	}
	shape->release();
}

void Renderer::D3D12PhysxSimulationApp::PlaySoundEffect(std::string soundName,const DirectX::SimpleMath::Vector3& emitterPosition, float volume)
{
	effect.release();
	effect = m_soundEffects[soundMap[soundName]]->CreateInstance(SoundEffectInstance_Use3D);

	listener.SetPosition(m_camera->GetPosition());
	listener.SetOrientation(m_camera->GetForwardDirection(), m_camera->GetUpDirection());

	emitter.SetPosition(emitterPosition);
	effect->SetVolume(volume);
	effect->Play();
	effect->Apply3D(listener, emitter, false);
}

void Renderer::D3D12PhysxSimulationApp::InitPhysics(bool interactive)
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(3);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = this;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane =
		PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	groundPlane->setName("Ground");
	gScene->addActor(*groundPlane);

	std::shared_ptr<Core::StaticMesh> plane = std::make_shared<Core::StaticMesh>();

	plane->Initialize(GeometryGenerator::PbrBox(300, 1, 300, L"DiamondPlate008C_4K-PNG_Albedo.dds", 50,1,50), m_device, m_commandList, DirectX::SimpleMath::Vector3(0.f, -1.f, -1.f),
		Material(1.f, 1.f, 1.f, 1.f),
		true, true, true, true, true, true);

	m_staticMeshes.push_back(plane);

	//gScene->setSimulationEventCallback(myCallback);

	CreateStack(PxTransform(PxVec3(0.f, 2.f, 0.f)), 10, 0.3f);
}

void Renderer::D3D12PhysxSimulationApp::InitScene()
{
	InitPhysics(true);

	using DirectX::SimpleMath::Vector3;
	using namespace Core;

	m_cubeMap = std::make_shared<Core::StaticMesh>();
	m_cubeMap->Initialize(GeometryGenerator::SimpleCubeMapBox(100.f), m_device, m_commandList);
	m_cubeMap->SetTexturePath(std::wstring(L"Outdoor") + L"EnvHDR.dds");

	m_screenMesh = std::make_shared<Core::StaticMesh>();
	m_screenMesh->Initialize(GeometryGenerator::Rectangle(2.f, L""), m_device, m_commandList);

	mCharacter = std::make_shared<Core::StaticMesh>();
	mCharacter->Initialize(GeometryGenerator::PbrSphere(0.5f, 100, 100,
		L"Metal048C_4K-PNG_Albedo.dds", 2.f, 2.f),
		m_device, m_commandList, Vector3(0.f, 0.f, 0.f),
		Material(1.f, 1.f, 1.f, 1.f),
		true /*AO*/, true /*Metallic*/, true /*Height*/, true /*Normal*/, true /*Roughness*/, false /*Tesslation*/);
	mCharacter->SetBoundingBoxHalfLength(0.5f);

}

void Renderer::D3D12PhysxSimulationApp::CreateDynamicBox(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& velocityDir, float velocity, float halfExtend)
{
	PxTransform t(PxVec3(position.x, position.y, position.z));
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, PxBoxGeometry(halfExtend, halfExtend, halfExtend),
		*gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	PxVec3 vel(velocityDir.x, velocityDir.y, velocityDir.z);
	dynamic->setLinearVelocity(vel * velocity);
	gScene->addActor(*dynamic);

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	PbrMeshData box = GeometryGenerator::PbrBox(halfExtend);
	std::shared_ptr<Core::StaticMesh> boxMesh = std::make_shared<Core::StaticMesh>();
	boxMesh->Initialize(box, m_device, m_commandList, position,
		Material(0.7f, 0.3f, 0.5f, 0.3f), false, false, false, false, false, false);
	m_staticMeshes.push_back(boxMesh);

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* pCmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);

	FlushCommandQueue();
}

void Renderer::D3D12PhysxSimulationApp::CreateDynamicSphere(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& velocityDir, float velocity, float halfExtend)
{

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	std::string str = "Ball";

	PbrMeshData sphere = GeometryGenerator::PbrSphere(halfExtend,100, 100, L"Metal048C_4K-PNG_Albedo.dds");
	std::shared_ptr<Core::StaticMesh> sphereMesh = std::make_shared<Core::StaticMesh>();
	sphere.m_name = str;
	sphereMesh->Initialize(sphere, m_device, m_commandList, position,
		Material(0.7f, 0.3f, 0.5f, 0.3f), true, true, true, true, true, false);
	m_staticMeshes.push_back(sphereMesh);

	PxTransform t(PxVec3(position.x, position.y, position.z));
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, PxSphereGeometry(halfExtend), *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	PxVec3 vel(velocityDir.x, velocityDir.y, velocityDir.z);
	dynamic->setLinearVelocity(vel * velocity);
	dynamic->setName(sphereMesh->m_name.c_str());
	gScene->addActor(*dynamic);


	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* pCmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);

	FlushCommandQueue();
}

void Renderer::D3D12PhysxSimulationApp::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	PxActor* actor1 = pairHeader.actors[0];
	PxActor* actor2 = pairHeader.actors[1];

	// 충돌된 액터들에 대한 로그 출력
	std::string name1 = actor1->getName();
	std::string name2 = actor2->getName();
	
	std::vector<PxContactPairPoint> contactPoints;

	if (name1 == "Ball" && name2 == "Ground") {
		for (PxU32 i = 0; i < nbPairs; i++)
		{
			PxU32 contactCount = pairs[i].contactCount;
			if (contactCount)
			{
				contactPoints.resize(contactCount);
				pairs[i].extractContacts(&contactPoints[0], contactCount);

				for (PxU32 j = 0; j < contactCount; j++)
				{
					if (contactPoints[j].impulse.magnitude() > 30.f)
					{
						gContactPositions.push_back(contactPoints[j].position);
						gContactImpulses.push_back(contactPoints[j].impulse);
					}
					//
				}
			}
		}
	}

}
