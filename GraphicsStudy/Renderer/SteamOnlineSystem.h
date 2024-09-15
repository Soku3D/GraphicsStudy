#pragma once
#pragma warning(disable : 4996)
#pragma warning(disable : 4828)
namespace Network {
	class SteamOnlineSystem;
}

#include <steam/steam_api.h>
#include <map>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>

#include <sstream>
#include <iostream>

#include "directxtk/SimpleMath.h"
#include "D3D12App.h"

namespace Network {

	struct PlayerData {
		DirectX::SimpleMath::Vector3 position;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar& position.x;
			ar& position.y;
			ar& position.z;
		}
	};

	struct GameState {
		PlayerData hostData;
		std::map<CSteamID, PlayerData> clientData;

		// Boost.Serialization을 위한 저장 함수
		template<class Archive>
		void save(Archive& ar, const unsigned int version) const {
			// hostData 저장
			ar& hostData;

			// CSteamID를 uint64_t로 변환하여 저장
			std::map<uint64_t, PlayerData> tempClientData;
			for (const auto& pair : clientData) {
				tempClientData[pair.first.ConvertToUint64()] = pair.second;
			}

			// 변환된 맵을 저장
			ar& tempClientData;
		}

		// Boost.Serialization을 위한 불러오기 함수
		template<class Archive>
		void load(Archive& ar, const unsigned int version) {
			// hostData 불러오기
			ar& hostData;

			// uint64_t에서 CSteamID로 변환하여 불러오기
			std::map<uint64_t, PlayerData> tempClientData;
			ar& tempClientData;

			clientData.clear();
			for (const auto& pair : tempClientData) {
				clientData[CSteamID(pair.first)] = pair.second;
			}
		}

		// 직렬화 함수로 save/load 호출
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			boost::serialization::split_member(ar, *this, version);
		}
	};

	class SteamOnlineSystem {
	public:
		SteamOnlineSystem(class Renderer::D3D12App* engine);
		~SteamOnlineSystem();

		STEAM_CALLBACK(SteamOnlineSystem, OnLobbyCreated, LobbyCreated_t);
		STEAM_CALLBACK(SteamOnlineSystem, OnP2PSessionRequest, P2PSessionRequest_t);
		STEAM_CALLBACK(SteamOnlineSystem, OnSteamNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t);
		STEAM_CALLBACK(SteamOnlineSystem, OnLobbyMatchList, LobbyMatchList_t);
		STEAM_CALLBACK(SteamOnlineSystem, OnLobbyEnter, LobbyEnter_t);

		/*void OnLobbyCreated(LobbyCreated_t* pCallback);
		void OnP2PSessionRequest(P2PSessionRequest_t* pCallback);
		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
		void OnLobbyMatchList(LobbyMatchList_t* pCallback);
		void OnLobbyEnter(LobbyEnter_t* pCallback);*/

		void CreateLobby(int maxMembers);
		void FindLobby();
		void EnterLobby(int lobbyIndex);

		void UpdateData(PlayerData& data);

		void Update();
		DirectX::SimpleMath::Vector3 GetClientData(int index);
		void SendGameState(const CSteamID& steamID);
		void ReadGameState(CSteamID& sender);

	private:
		bool isHost = false;

		HSteamNetConnection serverConnection;
		ISteamNetworkingSockets* networking;

		CSteamID mID;
		CSteamID hostID;
		CSteamID hostLobbyID;
		HSteamListenSocket listenSocket;
		std::vector<CSteamID> clientList;
		GameState mGameState;
		PlayerData mData = { {0,0,0} };

		class Renderer::D3D12App* pEngine;

		template<typename DataType>
		void SendData(const CSteamID& steamID, const DataType& data) {
			// GameState를 직렬화
			std::ostringstream oss;
			boost::archive::text_oarchive oa(oss);
			oa << data;

			// 직렬화된 데이터를 문자열로 변환
			std::string serializedData = oss.str();

			// Steam 네트워크를 통해 직렬화된 데이터 전송
			SteamNetworking()->SendP2PPacket(steamID, serializedData.c_str(), serializedData.size(), k_EP2PSendUnreliable);
		}
		template<typename DataType>
		void ReadData(CSteamID& sender, DataType& data, uint32& packetSize) {
			char* buffer = new char[packetSize];

			if (SteamNetworking()->ReadP2PPacket(buffer, packetSize, &packetSize, &sender)) {
				// 직렬화된 데이터를 문자열로 변환
				std::string serializedData(buffer, packetSize);

				// 역직렬화
				std::istringstream iss(serializedData);
				boost::archive::text_iarchive ia(iss);
				ia >> data;
			}

		}
	};
}