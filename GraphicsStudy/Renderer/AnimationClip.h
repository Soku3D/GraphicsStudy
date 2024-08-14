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

		string name;              // Name of this animation clip
		vector<vector<Key>> keys; // m_key[meshIdx][frameIdx]
		int numChannels;          // Number of meshes
		int numKeys;              // Number of frames of this animation clip
		double duration;          // Duration of animation in ticks
		double ticksPerSec;       // Frames per second
	};

	struct AnimationData {

		map<string, int32_t> meshNameToId;
		vector<string> meshIdToName;
		vector<Matrix> offsetMatrices;
		vector<Matrix> meshTransforms;
		vector<AnimationClip> clips;
		Matrix defaultTransform;
		Matrix accumulatedRootTransform = Matrix();
		Vector3 prevPos = Vector3(0.0f);

		Matrix Get(int clipId, int meshId, int frame) {
			return meshTransforms[meshId];
		}

		void Update(int clipId, int meshId, int frame) {
			
			auto& clip = clips[clipId];

			for (int boneId = 0; boneId < meshTransforms.size(); boneId++) {

				auto& keys = clip.keys[boneId];

				const Matrix parentMatrix = accumulatedRootTransform;

				auto key = keys.size() > 0
					? keys[frame % keys.size()]
					: AnimationClip::Key(); 


				if (frame != 0) {
					accumulatedRootTransform =
						Matrix::CreateTranslation(key.pos - prevPos) *
						accumulatedRootTransform;
				}
				else {
					auto temp = Vector3(0, 0, 0);
					accumulatedRootTransform.Translation(temp);
				}

				prevPos = key.pos;
				key.pos = Vector3(0.0f);


				meshTransforms[boneId] = key.GetTransform() * parentMatrix;
			}
		}
	};
}