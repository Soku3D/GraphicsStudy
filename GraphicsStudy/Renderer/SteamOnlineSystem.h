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
#include <sstream>
#include <iostream>

#include "directxtk/SimpleMath.h"
#include "D3D12App.h"

namespace Network {

	//struct PlayerData {
	//	DirectX::SimpleMath::Vector3 position;
	//};
	//struct GameState {
	//	PlayerData hostData;
	//	std::map<CSteamID, PlayerData> clientData;


	//	template<class Archive>
	//	void serialize(Archive& ar, const unsigned int version) {
	//		ar& hostData;
	//		ar& clientData;
	//	}
	//};
	struct PlayerData {
		DirectX::SimpleMath::Vector3 position;

		// Boost.Serialization을 위한 직렬화 함수
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

		// Boost.Serialization을 위한 직렬화/역직렬화 함수
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar& hostData;

			// 임시 맵을 사용하여 CSteamID -> uint64_t 변환
			std::map<uint64_t, PlayerData> tempClientData;
			if (Archive::is_saving::value) {
				// 직렬화 시 CSteamID -> uint64_t 변환
				for (const auto& pair : clientData) {
					tempClientData[pair.first.ConvertToUint64()] = pair.second;
				}
			}

			// 변환된 tempClientData를 직렬화 또는 역직렬화
			ar& tempClientData;

			if (Archive::is_loading::value) {
				// 역직렬화 시 uint64_t -> CSteamID 변환
				clientData.clear();
				for (const auto& pair : tempClientData) {
					clientData[CSteamID(pair.first)] = pair.second;
				}
			}
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
	};
}