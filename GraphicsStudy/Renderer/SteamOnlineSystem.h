#pragma once
#pragma warning(disable : 4996)
#pragma warning(disable : 4828)

#include <steam/steam_api.h>
#include <map>
#include <vector>

namespace Network {
	struct PlayerPosition {
		float x, y, z;
	};
	struct GameState {
		PlayerPosition hostPosition;
		std::map<CSteamID, PlayerPosition> clientPositions;
	};

	class SteamOnlineSystem {
	public:
		SteamOnlineSystem();
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
		void EnterLobby(int lobbyIndex);

		void Update();
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
		PlayerPosition mPosition = { 10.0f, 10.0f, 10.0f }; 
	};
}