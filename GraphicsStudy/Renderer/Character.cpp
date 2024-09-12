#include "Character.h"
#include "CameraComponent.h"
#include "StaticMesh.h"

Core::Character::Character()
	:Actor()
{
	mCamera = new CameraComponent();
	std::cout << "RightDirection : " << mRightDirection.x << ' ' << mRightDirection.y << ' ' << mRightDirection.z << '\n';

	m_delTheta = DirectX::XMConvertToRadians(0.2f);
	m_delSine = sin(m_delTheta / 2.f);
	m_delCosine = cos(m_delTheta / 2.f);
}

size_t Core::Character::GetMeshCount() const
{
	if (mStaticMesh != nullptr) {
		return mStaticMesh->meshCount;
	}
	return 0;
}

void Core::Character::SetRotation(int deltaX, int deltaY) {
	m_xTheta += deltaY * mCamera->m_aspectRatio * m_delTheta;
	m_yTheta += deltaX * m_delTheta;

	if (m_xTheta >= DirectX::XM_PIDIV2 - 0.001f) {
		m_xTheta = DirectX::XM_PIDIV2 - 0.001f;
	}
	if (m_xTheta <= -DirectX::XM_PIDIV2 + 0.001f) {
		m_xTheta = -DirectX::XM_PIDIV2 + 0.001f;
	}

	/*if (m_yTheta >= -DirectX::XM_2PI) {
		m_yTheta -= DirectX::XM_2PI;
	}
	if (m_yTheta <= -DirectX::XM_2PI) {
		m_yTheta += DirectX::XM_2PI;
	}*/
}

void Core::Character::SetCameraAspectRatio(float ratio)
{
	mCamera->SetAspectRatio(ratio);
}

DirectX::SimpleMath::Matrix Core::Character::GetViewMatrix()
{
	if (mCamera != nullptr) {
		return mCamera->GetViewMatrix();
	}

	return DirectX::SimpleMath::Matrix();
}

DirectX::SimpleMath::Matrix Core::Character::GetProjMatrix()
{
	if (mCamera != nullptr) {
		return mCamera->GetProjMatrix();
	}

	return DirectX::SimpleMath::Matrix();
}

DirectX::SimpleMath::Vector3 Core::Character::GetViewDirection()
{
	return mViewDirection;
}

DirectX::SimpleMath::Vector3 Core::Character::GetForwardDirection()
{
	return mForwardDirection;
}

DirectX::SimpleMath::Vector3 Core::Character::GetUpDirection()
{
	return mUpDirection;
}

DirectX::SimpleMath::Vector3 Core::Character::GetPosition()
{
	if (mCamera != nullptr) {
		return mCamera->GetPosition();
	}

	return DirectX::SimpleMath::Vector3();
}

void Core::Character::Update(float deltaTime)
{
	using DirectX::SimpleMath::Matrix;

	if (mStaticMesh != nullptr) 
	{
		//std::cout << "Model Rotate : " << m_yTheta << ' ';
		Matrix Model = DirectX::XMMatrixRotationY(m_yTheta) * DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		mStaticMesh->UpdateWorldRow(Model);
	    mStaticMesh->Update(deltaTime);
	}

	if (mCamera != nullptr) 
	{
		mCamera->Update(mPosition, m_xTheta, m_yTheta);
		//mCamera->SetPositionAndDirection(mPosition + XMFLOAT3(0, 0, -1), XMFLOAT3(0, 0, 1));
	}

}

void Core::Character::Render(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, int index)
{
	Actor::Render(deltaTime, commandList, index);
}

void Core::Character::RenderBoundingBox(float deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	Actor::RenderBoundingBox(deltaTime, commandList);
}

void Core::Character::RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int index)
{
	Actor::RenderNormal(deltaTime, commandList, bUseModelMat, index);
}

void Core::Character::RotateDirection() {
	using DirectX::SimpleMath::Vector3;
	/*	if (mViewDirection.Dot(Vector3(0.f, 1.f, 0.f)) < 0.99f && mViewDirection.Dot(Vector3(0.f, -1.f, 0.f)) > -0.99f) {
		mViewDirection = Vector3::Transform(mViewDirection, DirectX::XMMatrixRotationQuaternion(m_quaternion));
		mViewDirection.Normalize();
		m_quaternion = DirectX::SimpleMath::Quaternion();

		}*/
	DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixRotationX(m_xTheta) * DirectX::XMMatrixRotationY(m_yTheta);
	mViewDirection = Vector3::Transform(mStandardDirection, mat);
	mViewDirection.Normalize();

	mForwardDirection = Vector3::Transform(mStandardDirection, DirectX::XMMatrixRotationY(m_yTheta));
	mForwardDirection.Normalize();

	mRightDirection = mUpDirection.Cross(mForwardDirection);
	mRightDirection.Normalize();
}

void Core::Character::MoveUp(float deltaTime)
{
	/*std::cout << "MoveUp";
	mPosition += (mVelocity * deltaTime) * mUpDirection;*/
}

void Core::Character::MoveDown(float deltaTime)
{
	//mPosition += (mVelocity * deltaTime) * -mUpDirection;
}

void Core::Character::MoveRight(float deltaTime)
{
	mPosition += (mVelocity * deltaTime) * mRightDirection;
}

void Core::Character::MoveLeft(float deltaTime)
{
	mPosition += (mVelocity * deltaTime) * -mRightDirection;
}

void Core::Character::MoveForward(float deltaTime)
{
	mPosition += (mVelocity * deltaTime) * mForwardDirection;
}

void Core::Character::MoveBackward(float deltaTime)
{
	mPosition += (mVelocity * deltaTime) * -mForwardDirection;
}
