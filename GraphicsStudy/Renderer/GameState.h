#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <steam/steam_api.h>
#include "directxtk/SimpleMath.h"

struct PlayerData {
	DirectX::SimpleMath::Vector3 position;
	float yTheta;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& position.x;
		ar& position.y;
		ar& position.z;

		ar& yTheta;
	}
};

struct GameState {
	PlayerData hostData;
	std::map<CSteamID, PlayerData> clientData;

	// Boost.Serialization을 위한 직렬화 함수
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& hostData;

		// CSteamID를 uint64_t로 변환하여 저장
		std::map<uint64_t, PlayerData> tempClientData;
		if (Archive::is_saving::value) {
			for (const auto& pair : clientData) {
				tempClientData[pair.first.ConvertToUint64()] = pair.second;
			}
			ar& tempClientData;
		}
		else {
			ar& tempClientData;
			clientData.clear();
			for (const auto& pair : tempClientData) {
				clientData[CSteamID(pair.first)] = pair.second;
			}
		}
	}
};
BOOST_CLASS_VERSION(PlayerData, 1)
BOOST_CLASS_VERSION(GameState, 1)