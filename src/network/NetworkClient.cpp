#include "NetworkClient.hpp"

#include <iostream>
#include <sstream>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace LibreGE {

static void CloseSocket(
    int socket
) {
#if defined(_WIN32)
    closesocket(socket);
#else
    close(socket);
#endif
}

NetworkClient::NetworkClient() {
#if defined(_WIN32)

    WSADATA data;

    WSAStartup(
        MAKEWORD(2, 2),
        &data
    );

#endif
}

NetworkClient::~NetworkClient() {
    Disconnect();

#if defined(_WIN32)
    WSACleanup();
#endif
}

bool NetworkClient::Connect(
    const std::string& host,
    int port,
    const std::string& name
) {
    if (m_Connected) {
        return true;
    }

    addrinfo hints {};
    addrinfo* result = nullptr;

    hints.ai_family =
        AF_UNSPEC;

    hints.ai_socktype =
        SOCK_STREAM;

    const std::string portString =
        std::to_string(port);

    if (
        getaddrinfo(
            host.c_str(),
            portString.c_str(),
            &hints,
            &result
        ) != 0
    ) {
        std::cerr
            << "[Network] getaddrinfo failed\n";

        return false;
    }

    for (
        addrinfo* current = result;
        current != nullptr;
        current = current->ai_next
    ) {
        m_Socket =
            static_cast<int>(
                socket(
                    current->ai_family,
                    current->ai_socktype,
                    current->ai_protocol
                )
            );

        if (m_Socket < 0) {
            continue;
        }

        if (
            connect(
                m_Socket,
                current->ai_addr,
                current->ai_addrlen
            ) == 0
        ) {
            break;
        }

        CloseSocket(
            m_Socket
        );

        m_Socket = -1;
    }

    freeaddrinfo(
        result
    );

    if (m_Socket < 0) {
        std::cerr
            << "[Network] Could not connect to "
            << host
            << ":"
            << port
            << '\n';

        return false;
    }

    m_Connected =
        true;

    SendLine(
        "HELLO " + name
    );

    m_ReceiveThread =
        std::thread(
            &NetworkClient::ReceiveLoop,
            this
        );

    std::cout
        << "[Network] Connected to "
        << host
        << ":"
        << port
        << '\n';

    return true;
}

void NetworkClient::Disconnect() {
    if (!m_Connected.exchange(false)) {
        return;
    }

#if !defined(_WIN32)
    shutdown(
        m_Socket,
        SHUT_RDWR
    );
#else
    shutdown(
        m_Socket,
        SD_BOTH
    );
#endif

    CloseSocket(
        m_Socket
    );

    m_Socket = -1;

    if (
        m_ReceiveThread.joinable()
    ) {
        m_ReceiveThread.join();
    }

    std::lock_guard lock(
        m_PlayerMutex
    );

    m_Players.clear();
}

bool NetworkClient::IsConnected() const {
    return m_Connected;
}

bool NetworkClient::SendLine(
    const std::string& line
) {
    if (!m_Connected) {
        return false;
    }

    const std::string packet =
        line + "\n";

    const char* data =
        packet.data();

    std::size_t remaining =
        packet.size();

    while (remaining > 0) {
        const int written =
            static_cast<int>(
                send(
                    m_Socket,
                    data,
                    static_cast<int>(
                        remaining
                    ),
                    0
                )
            );

        if (written <= 0) {
            return false;
        }

        data += written;
        remaining -=
            static_cast<std::size_t>(
                written
            );
    }

    return true;
}

void NetworkClient::SendPosition(
    float x,
    float y
) {
    std::ostringstream message;

    message
        << "POS "
        << x
        << " "
        << y;

    SendLine(
        message.str()
    );
}

void NetworkClient::ReceiveLoop() {
    std::string buffer;

    char chunk[2048];

    while (m_Connected) {
        const int received =
            static_cast<int>(
                recv(
                    m_Socket,
                    chunk,
                    sizeof(chunk),
                    0
                )
            );

        if (received <= 0) {
            break;
        }

        buffer.append(
            chunk,
            received
        );

        std::size_t position;

        while (
            (
                position =
                    buffer.find('\n')
            )
            !=
            std::string::npos
        ) {
            std::string line =
                buffer.substr(
                    0,
                    position
                );

            buffer.erase(
                0,
                position + 1
            );

            HandleLine(
                line
            );
        }
    }

    m_Connected =
        false;
}

void NetworkClient::HandleLine(
    const std::string& line
) {
    std::istringstream input(
        line
    );

    std::string command;

    input >> command;

    if (command == "WELCOME") {
        int id;

        input >> id;

        m_PlayerID =
            id;

        std::cout
            << "[Network] Player ID: "
            << id
            << '\n';

        return;
    }

    if (command == "PLAYER") {
        RemotePlayer player;

        input
            >> player.id
            >> player.name
            >> player.x
            >> player.y;

        std::lock_guard lock(
            m_PlayerMutex
        );

        m_Players[
            player.id
        ] = player;

        return;
    }

    if (command == "REMOVE") {
        int id;

        input >> id;

        std::lock_guard lock(
            m_PlayerMutex
        );

        m_Players.erase(
            id
        );
    }
}

int NetworkClient::GetPlayerID() const {
    return m_PlayerID;
}

std::unordered_map<
    int,
    RemotePlayer
>
NetworkClient::GetPlayers() const {
    std::lock_guard lock(
        m_PlayerMutex
    );

    return m_Players;
}

}
