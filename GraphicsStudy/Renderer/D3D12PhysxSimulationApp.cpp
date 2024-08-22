#include "D3D12PhysxSimulationApp.h"

#include "GeometryGenerator.h"

Renderer::D3D12PhysxSimulationApp::D3D12PhysxSimulationApp(const int& width, const int& height)
	:D3D12PassApp(width, height)
{
    bUseGUI = true;
    bRenderCubeMap = true;
    bRenderMeshes = true;
    bRenderFbx = false;
    bRenderNormal = false;
}

bool Renderer::D3D12PhysxSimulationApp::Initialize()
{
	if (!D3D12PassApp::Initialize())
		return false;

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
    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);

    // gScene->getActors()
    // PxGeometryType::eBOX: , case PxGeometryType::eSPHERE:

    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
        PxActorTypeFlag::eRIGID_STATIC);

    std::vector<PxRigidActor*> actors(nbActors);
    gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC |
        PxActorTypeFlag::eRIGID_STATIC,
        reinterpret_cast<PxActor**>(&actors[0]), nbActors);

    PxShape* shapes[MAX_NUM_ACTOR_SHAPES];

    int count = 0;

    for (PxU32 i = 0; i < nbActors; i++) {

        const PxU32 nbShapes = actors[i]->getNbShapes();
        PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
        actors[i]->getShapes(shapes, nbShapes);
        for (PxU32 j = 0; j < nbShapes; j++) {
            const PxMat44 shapePose(
                PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));

            if (actors[i]->is<PxRigidDynamic>()) {

                bool speeping = actors[i]->is<PxRigidDynamic>() &&
                    actors[i]->is<PxRigidDynamic>()->isSleeping();
                // cout << i << " : " << shapePose.getPosition().y << " sleeping
                // "
                //      << speeping << endl;

                m_staticMeshes[count]->UpdateWorldRow(DirectX::SimpleMath::Matrix(shapePose.front()) *
                    DirectX::SimpleMath::Matrix::CreateScale(1.00f));
               
                count++;
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

physx::PxRigidDynamic* Renderer::D3D12PhysxSimulationApp::createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity)
{
    physx::PxRigidDynamic* dynamic =
        PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    gScene->addActor(*dynamic);
    return dynamic;
}

void Renderer::D3D12PhysxSimulationApp::createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
    std::vector<PbrMeshData> box = { GeometryGenerator::PbrBox(halfExtent) };

    PxShape* shape = gPhysics->createShape(
        PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);

    for (PxU32 i = 2; i < size; i++) {
        for (PxU32 j = 0; j < size - i; j++) {
            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i),
                PxReal(i * 2 + 1), 0) *
                halfExtent);
            PxRigidDynamic* body =
                gPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            gScene->addActor(*body);

         /*   auto m_newObj =
                std::make_shared<Model>(m_device, m_context, box);
            m_newObj->m_materialConsts.GetCpu().albedoFactor =
                Vector3(0.5f);
            AppBase::m_basicList.push_back(m_newObj);
            this->m_objects.push_back(m_newObj);*/
        }
    }
    shape->release();
}

void Renderer::D3D12PhysxSimulationApp::InitPhysics(bool interactive)
{
    gFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport =
        PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation,
        PxTolerancesScale(), true, gPvd);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(12);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
            true);
    }
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic* groundPlane =
        PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);


    std::vector<PbrMeshData> box = { GeometryGenerator::PbrUseTesslationBox(0.2f) };
    PxTransform t = PxTransform(PxVec3(0, 1.0f, 0.f));
    PxShape* shape =
        gPhysics->createShape(PxBoxGeometry(0.2f, 0.2f, 0.2f), *gMaterial);

    PxTransform localTm(PxVec3(0, 10, 0) * 0.2f);
    PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
    body->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
    gScene->addActor(*body);

  /*  auto m_newObj = std::make_shared<Model>(m_device, m_context, box);
    m_newObj->m_materialConsts.GetCpu().albedoFactor = Vector3(0.5f);
    AppBase::m_basicList.push_back(m_newObj);
    this->m_objects.push_back(m_newObj);*/

    std::shared_ptr<Core::StaticMesh> boxMesh = std::make_shared<Core::StaticMesh>();
    boxMesh->Initialize(box[0], m_device, m_commandList, DirectX::SimpleMath::Vector3::Zero,
        Material(0.7f, 0.3f, 0.5f, 0.3f), false, false, false, false, false, false);
    m_staticMeshes.push_back(boxMesh);

    shape->release();

}

void Renderer::D3D12PhysxSimulationApp::InitScene()
{
	InitPhysics(true);
    D3D12PassApp::InitScene();
}
