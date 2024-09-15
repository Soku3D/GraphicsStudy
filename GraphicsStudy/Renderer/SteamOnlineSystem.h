#pragma once
#pragma warning(disable : 4996)
#pragma warning(disable : 4828)
namespace Network {
	class SteamOnlineSystem;
}

#include <steam/steam_api.h>
#include <map>
#include <vector>
#include "directxtk/SimpleMath.h"
#include "D3D12App.h"

namespace Network {

	struct PlayerData {
		DirectX::SimpleMath::Vector3 position;
	};
	struct GameState {
		PlayerData hostData;
		std::map<CSteamID, PlayerData> clientData;
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
		//void UpdatePosition();

	private:
		bool isHost = false;

		HSteamNetConnection serverConnection;
		ISteamNetworkingSockets* networking;

		CSteamID hostID;
		CSteamID hostLobbyID;
		HSteamListenSocket listenSocket;
		std::vector<CSteamID> clientList;
		GameState mGameState;
		PlayerData mData = { {0,0,0} };

		class Renderer::D3D12App* pEngine;
	};
}