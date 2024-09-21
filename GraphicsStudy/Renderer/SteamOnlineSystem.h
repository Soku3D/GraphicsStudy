#pragma once
#pragma warning(disable : 4996)
#pragma warning(disable : 4828)
namespace Network {
	class SteamOnlineSystem;
}


#include <map>
#include <vector>

#include <sstream>
#include <iostream>

#include "D3D12App.h"
#include "GameState.h"

namespace Network {

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
		PlayerData GetClientData(int index);

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
			SteamNetworking()->SendP2PPacket(steamID, serializedData.c_str(), (uint32)serializedData.size(), k_EP2PSendUnreliable);
		}

		template<typename DataType>
		void ReadData(CSteamID& sender, DataType& data, uint32& packetSize) {
			if (packetSize == 0) {
				return; // 패킷 크기가 0인 경우 처리하지 않음
			}

			std::vector<char> buffer(packetSize);

			if (SteamNetworking()->ReadP2PPacket(buffer.data(), packetSize, &packetSize, &sender)) {
				// 직렬화된 데이터를 문자열로 변환
				std::string serializedData(buffer.data(), packetSize);

				// 역직렬화
				std::istringstream iss(serializedData);
				try {
					boost::archive::text_iarchive ia(iss);
					ia >> data;
				}
				catch (const boost::archive::archive_exception& e) {
					std::cerr << "역직렬화 실패: " << e.what() << std::endl;
					return;
				}
			}
		}

	};
}

