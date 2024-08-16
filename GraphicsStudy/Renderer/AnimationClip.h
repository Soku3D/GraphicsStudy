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

		map<string, int32_t> meshNameToId;
		vector<string> meshIdToName;
		vector<Matrix> offsetMatrices;
		vector<Matrix> meshTransforms;
		vector<AnimationClip> clips;
		Matrix defaultTransform;
		Matrix accumulatedRootTransform = Matrix();
		Vector3 prevPos = Vector3(0.0f);

		Matrix Get(int meshId) {
			return meshTransforms[meshId];
		}
		
		void Update(int frame) {

			auto& clip = clips[0];

			for (int meshId = 0; meshId < meshTransforms.size(); meshId++) {

				auto& keys = clip.keys[meshId];
				auto key = keys[frame % keys.size()];
				meshTransforms[meshId] = key.GetTransform();
			}
		}
	};
}