#include "Actor.h"
#include "StaticMesh.h"

Core::Actor::Actor() :
	mPosition(DirectX::SimpleMath::Vector3(0, 0, 0)),
	mViewDirection(DirectX::SimpleMath::Vector3(0, 0, 1)),
	mUpDirection(DirectX::SimpleMath::Vector3(0.f, 1.f, 0.f)),
	mRightDirection(DirectX::SimpleMath::Vector3(1.f, 0.f, 0.f))
{
	mStandardDirection = mViewDirection;
}

void Core::Actor::Update(float deltaTime)
{
}

void Core::Actor::Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, int index)
{
	if (mStaticMesh != nullptr)
	{
		mStaticMesh->Render(deltaTime, commandList, true, index);
	}
}

void Core::Actor::RenderBoundingBox(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	if (mStaticMesh != nullptr)
	{
		mStaticMesh->RenderBoundingBox(deltaTime, commandList);
	}
}

void Core::Actor::RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int index)
{
	if (mStaticMesh != nullptr)
	{
		mStaticMesh->RenderNormal(deltaTime, commandList, bUseModelMat, index);
	}
}




void Core::Actor::SetStaticMeshComponent(StaticMesh* staticMesh)
{
	mStaticMesh = staticMesh;
}

std::wstring Core::Actor::GetTexturePath(int index) const
{
	return mStaticMesh->GetTexturePath(index);
}

void Core::Actor::SetPosition(const DirectX::SimpleMath::Vector3& position)
{
	mPosition = position;
}

void Core::Actor::SetForwardDirection(const DirectX::SimpleMath::Vector3& direction)
{
	using DirectX::SimpleMath::Vector3;

	mForwardDirection = direction;

	Vector3 v0 = Vector3(0, mForwardDirection.y, mForwardDirection.z);
	Vector3 v1 = Vector3(mForwardDirection.x, 0, mForwardDirection.z);
	v0.Normalize();
	v1.Normalize();
	float cosTheta1 = abs(v0.Dot(Vector3(0, 0, 1)));
	float cosTheta2 = v1.Dot(Vector3(0, 0, 1));
	m_xTheta = acos(cosTheta1);
	m_yTheta = acos(cosTheta2);
	if (v0.y > 0) {
		m_xTheta *= -1.f;
	}
	if (v1.x < 0) {
		m_yTheta *= -1.f;
	}
	m_delTheta = DirectX::XMConvertToRadians(0.2f);
	m_delSine = sin(m_delTheta / 2.f);
	m_delCosine = cos(m_delTheta / 2.f);

	mViewDirection = Vector3::Transform(mStandardDirection, DirectX::XMMatrixRotationX((float)m_xTheta));
	mViewDirection = Vector3::Transform(mViewDirection, DirectX::XMMatrixRotationY((float)m_yTheta));
	mViewDirection.Normalize();

	mForwardDirection = Vector3::Transform(mStandardDirection, DirectX::XMMatrixRotationY((float)m_yTheta));;
	mForwardDirection.Normalize();

	mRightDirection = mUpDirection.Cross(mForwardDirection);
	mRightDirection.Normalize();
}

void Core::Actor::SetVelocity(const float& velocity)
{
	mVelocity = velocity;
}

DirectX::SimpleMath::Matrix Core::Actor::GetTransformMatrix()
{
	if (mStaticMesh != nullptr) {
		return mStaticMesh->GetTransformMatrix();
	}
	return DirectX::SimpleMath::Matrix();
}

D3D12_GPU_VIRTUAL_ADDRESS Core::Actor::GetBlas(int index)
{
	if (mStaticMesh != nullptr) {
		return mStaticMesh->GetBlas(index);
	}
	return D3D12_GPU_VIRTUAL_ADDRESS();
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::Actor::GetIndexCpuHandle(int index)
{
	if (mStaticMesh != nullptr) {
		return mStaticMesh->GetIndexCpuHandle(index);
	}
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

PrimitiveConstantBuffer Core::Actor::GetprimitiveConstantData()
{

	if (mStaticMesh != nullptr) {
		return mStaticMesh->m_primitiveConstantData;
	}

	return PrimitiveConstantBuffer();
}
