#include "SteamOnlineSystem.h"



//Network::SteamOnlineSystem::SteamOnlineSystem(Renderer::D3D12App* engine)
//    :pEngine(engine)
//{
//    if (!SteamAPI_Init()) {
//        std::cerr << "Steam API 초기화 실패!" << std::endl;
//    }
//    else {
//        std::cout << "Steam API 초기화 성공!" << std::endl;
//
//    }
//    networking = SteamNetworkingSockets();
//}

Network::SteamOnlineSystem::SteamOnlineSystem(Renderer::D3D12App* engine)
    :pEngine(engine)
{
    if (!SteamAPI_Init()) {
        std::cerr << "Steam API 초기화 실패!" << std::endl;
    }
    else {
        std::cout << "Steam API 초기화 성공!" << std::endl;

    }
    networking = SteamNetworkingSockets();
}

Network::SteamOnlineSystem::~SteamOnlineSystem()
{
    //if(serverConnection != k_HSteamNetConnection_Invalid)
    SteamAPI_Shutdown();
}


void Network::SteamOnlineSystem::OnLobbyCreated(LobbyCreated_t* pCallback)
{
    if (pCallback->m_eResult == k_EResultOK) {
        isHost = true;
        std::cout << "Create Lobby Complete" << std::endl;
        //SteamMatchmaking()->SetLobbyData(pCallback->m_ulSteamIDLobby, "owner_name", SteamFriends()->GetPersonaName());
    }
    else {
        std::cout << "Failed to Create Lobby" << std::endl;
    }
}

void Network::SteamOnlineSystem::OnP2PSessionRequest(P2PSessionRequest_t* pCallback)
{
    SteamNetworking()->AcceptP2PSessionWithUser(pCallback->m_steamIDRemote);
    std::cout << "P2P 연결 요청을 승인합니다." << std::endl;
}

void Network::SteamOnlineSystem::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected) {
        std::cout << "Connected." << std::endl;
    }
    else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting) {
        std::cout << "Connecting ..." << std::endl;
    }

    else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
        std::cout << "ClosedByPeer" << std::endl;
    }
    else if(pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)   {
        std::cout << "ProblemDetectedLocally" << std::endl;
        //TryConnectToServer();
    }
    else {
        std::cout << "Disconnected" << " State : " << pInfo->m_info.m_eState << std::endl;
    }
}

void Network::SteamOnlineSystem::OnLobbyMatchList(LobbyMatchList_t* pCallback)
{
    if (pCallback->m_nLobbiesMatching > 0) {
        std::cout << "로비 " << pCallback->m_nLobbiesMatching<< "개를 찾았습니다."<< std::endl;
        for (uint32 i = 0; i < pCallback->m_nLobbiesMatching; i++)
        {
            CSteamID lobbyID = SteamMatchmaking()->GetLobbyByIndex(i);
            //CSteamID ownerID = SteamMatchmaking()->GetLobbyOwner(lobbyID);
            std::string ownerName = SteamMatchmaking()->GetLobbyData(lobbyID, "owner_name");
            std::cout << "Lobby " << i << " Owner : " << ownerName << std::endl;
        }

        //TODO index 선택 할 수 있도록
        EnterLobby(0);
    }
    else {
        std::cout << "로비를 찾지 못했습니다." << std::endl;
    }
}

void Network::SteamOnlineSystem::OnLobbyEnter(LobbyEnter_t* pCallback)
{
    if(!isHost)
    {
        std::cout << "로비 참가 성공." << std::endl;
        int memberCount = SteamMatchmaking()->GetNumLobbyMembers(pCallback->m_ulSteamIDLobby);

        for (int i = 0; i < memberCount; i++) {
            CSteamID memberSteamID = SteamMatchmaking()->GetLobbyMemberByIndex(pCallback->m_ulSteamIDLobby, i);

            // 멤버의 닉네임 가져오기
            const char* memberName = SteamFriends()->GetFriendPersonaName(memberSteamID);

            std::cout << "멤버 Steam ID: " << memberSteamID.ConvertToUint64() << std::endl;
            std::cout << "멤버 닉네임: " << memberName << std::endl;
            if (memberSteamID == SteamUser()->GetSteamID()) {
                continue;
            }
        }
        hostID = SteamMatchmaking()->GetLobbyOwner(pCallback->m_ulSteamIDLobby);
        SteamNetworkingIdentity identity;
        identity.SetSteamID(hostID);

        serverConnection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, nullptr);

        if (serverConnection == k_HSteamNetConnection_Invalid) {
            std::cerr << "서버 P2P 연결 실패!" << std::endl;
        }
        else {
            std::cout << "서버 P2P 연결 성공!" << std::endl;
        }
    }
}

void Network::SteamOnlineSystem::CreateLobby(int maxMembers)
{
    SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxMembers);
}

void Network::SteamOnlineSystem::FindLobby() {
    SteamMatchmaking()->RequestLobbyList();
}

void Network::SteamOnlineSystem::EnterLobby(int lobbyIndex)
{
    hostLobbyID = SteamMatchmaking()->GetLobbyByIndex(lobbyIndex);
    hostID = SteamMatchmaking()->GetLobbyOwner(hostLobbyID);
    SteamMatchmaking()->JoinLobby(hostLobbyID);
}
void Network::SteamOnlineSystem::UpdateData(PlayerData& data) {
    mData = data;
}

void Network::SteamOnlineSystem::Update()
{
    SteamAPI_RunCallbacks();
    if(networking!=nullptr)
    {
        networking->RunCallbacks();
        if (isHost) {
            uint32 size;
            PlayerData clientData;
            while (SteamNetworking()->IsP2PPacketAvailable(&size)) {
                CSteamID clientSteamID;
                SteamNetworking()->ReadP2PPacket(&clientData, sizeof(PlayerData), &size, &clientSteamID);
                mGameState.clientData[clientSteamID] = clientData;  // 최신 패킷만 저장
                
                if (std::find(clientList.begin(), clientList.end(), clientSteamID) == clientList.end()) {
                    clientList.emplace_back(clientSteamID);
                    // 새로운 플레이어 등록
                    //pEngine->AddPlayer();
                    pEngine->addPlayerCount++;
                }
            }
            for (size_t i = 0; i < clientList.size(); ++i)
            {
                if (clientList[i].IsValid()) {
                    //// 호스트 데이터
                    //mGameState.hostData = mData;
                    //UINT stateSize = sizeof(GameState);
                    //SteamNetworking()->SendP2PPacket(clientList[i], &mGameState.hostData, sizeof(PlayerData), k_EP2PSendUnreliable);
                    mGameState.hostData = mData;
                    SendGameState(clientList[i]);
                }
            }

        }
        else {
            if (hostID.IsValid()) {
                SteamNetworking()->SendP2PPacket(hostID, &mData, sizeof(PlayerData), k_EP2PSendUnreliable);
            }
            CSteamID id;
            ReadGameState(id);
                        
        }
    }
}

DirectX::SimpleMath::Vector3 Network::SteamOnlineSystem::GetClientData(int index) {
    return mGameState.clientData[clientList[index]].position;
}


void Network::SteamOnlineSystem::SendGameState(const CSteamID& steamID) {
    // GameState를 직렬화
    std::ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << mGameState;

    // 직렬화된 데이터를 문자열로 변환
    std::string serializedData = oss.str();

    // Steam 네트워크를 통해 직렬화된 데이터 전송
    SteamNetworking()->SendP2PPacket(steamID, serializedData.c_str(), serializedData.size(), k_EP2PSendReliable);
}

void Network::SteamOnlineSystem::ReadGameState(CSteamID& sender) {
    uint32 packetSize;
    char buffer[4096];
    
    while (SteamNetworking()->IsP2PPacketAvailable(&packetSize)) {
        // 패킷 읽기
        CSteamID remoteSteamID;
        if (SteamNetworking()->ReadP2PPacket(buffer, sizeof(buffer), &packetSize, &remoteSteamID)) {
            // 직렬화된 데이터를 문자열로 변환
            std::string serializedData(buffer, packetSize);

            // 역직렬화
            std::istringstream iss(serializedData);
            boost::archive::text_iarchive ia(iss);
            ia >> mGameState;
    }
}
