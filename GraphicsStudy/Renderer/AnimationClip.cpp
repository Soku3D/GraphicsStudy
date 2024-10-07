#include "AnimationClip.h"

namespace Animation {
	Matrix AnimationData::Get(int boneId)
	{
		return defaultTransform.Invert() * offsetMatrices[boneId] *
			boneTransforms[boneId] * defaultTransform;
	}

	void AnimationData::Update(int frame)
	{

		auto& clip = clips[0];
		for (int boneId = 0; boneId < boneTransforms.size(); boneId++) {

			auto& keys = clip.keys[boneId];
			auto key = keys.size() > 0 ? keys[frame % keys.size()]
				: AnimationClip::Key();
			int parentId = boneParentsId[boneId];
			Matrix parentMatrix = parentId > 0 ? boneTransforms[parentId]
				: accumulatedRootTransform;
			if (parentId < 0) {
				if (frame != 0) {
					accumulatedRootTransform =
						Matrix::CreateTranslation(key.pos - prevPos) *
						accumulatedRootTransform;
				}
				else {
					auto temp = accumulatedRootTransform.Translation();
					temp.y = key.pos.y;
					accumulatedRootTransform.Translation(temp);
				}

				prevPos = key.pos;
				key.pos = Vector3(0.0f);
			}
			boneTransforms[boneId] = key.GetTransform() * parentMatrix;

		}
	}

	Matrix AnimationClip::Key::GetTransform()
	{
		return Matrix::CreateScale(scale) *
			Matrix::CreateFromQuaternion(rot) *
			Matrix::CreateTranslation(pos);
	}

}