#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace LibreGE {

struct RemotePlayer {
    int id = -1;
    std::string name;

    float x = 0.0f;
    float y = 0.0f;
};

class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();

    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    bool Connect(
        const std::string& host,
        int port,
        const std::string& name
    );

    void Disconnect();

    bool IsConnected() const;

    void SendPosition(
        float x,
        float y
    );

    int GetPlayerID() const;

    std::unordered_map<int, RemotePlayer>
    GetPlayers() const;

private:
    void ReceiveLoop();

    void HandleLine(
        const std::string& line
    );

    bool SendLine(
        const std::string& line
    );

private:
    int m_Socket = -1;

    std::atomic<bool> m_Connected {
        false
    };

    std::atomic<int> m_PlayerID {
        -1
    };

    std::thread m_ReceiveThread;

    mutable std::mutex m_PlayerMutex;

    std::unordered_map<
        int,
        RemotePlayer
    > m_Players;
};

}
