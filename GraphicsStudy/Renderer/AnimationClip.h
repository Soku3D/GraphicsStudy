#pragma once

#include <directxtk/SimpleMath.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace Animation {

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Quaternion;
	using DirectX::SimpleMath::Vector3;
	using std::map;
	using std::string;
	using std::vector;

	struct AnimationClip {

		struct Key {
			Vector3 pos = Vector3(0.0f);
			Vector3 scale = Vector3(1.0f);
			Quaternion rot = Quaternion();

			Matrix GetTransform() {
				return Matrix::CreateScale(scale) *
					Matrix::CreateFromQuaternion(rot) *
					Matrix::CreateTranslation(pos);
			}
		};

		string name;
		vector<vector<Key>> keys; // m_key[meshIdx][frameIdx]
		int numChannels;
		int numKeys;
		double duration;
		double ticksPerSec;
	};

	struct AnimationData {

		map<string, int32_t> boneNameToId;
		vector<int32_t> boneParentsId;
		vector<string> boneIdToName;
		vector<Matrix> offsetMatrices;
		vector<Matrix> boneTransforms;
		vector<AnimationClip> clips;
		Matrix defaultTransform;
		Matrix accumulatedRootTransform = Matrix();
		Vector3 prevPos = Vector3(0.0f);

		Matrix Get(int boneId) {
			return defaultTransform.Invert() * offsetMatrices[boneId] *
				boneTransforms[boneId] * defaultTransform;
		}

		void Update(int frame) {

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
	};
}