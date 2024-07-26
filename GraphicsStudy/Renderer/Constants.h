#pragma once

__declspec(align(256)) struct GlobalVertexConstantData
{
	DirectX::SimpleMath::Matrix ViewMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix ProjMat = DirectX::SimpleMath::Matrix();
};

__declspec(align(256)) struct ObjectConstantData {
	DirectX::SimpleMath::Matrix Model = DirectX::SimpleMath::Matrix();

};
