#include <RakPeerInterface.h>
#include <BitStream.h>
#include <MessageIdentifiers.h>
#include <RakNetTypes.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <string>

using namespace RakNet;

std::random_device rd;
std::mt19937 rng(rd());

void sendUnconnectedPing(RakPeerInterface* peer, const char* ip, unsigned short port) {
    BitStream bs;
    bs.Write((MessageID)ID_UNCONNECTED_PING);
    uint64_t timestamp = std::uniform_int_distribution<uint64_t>(1000000000, 9999999999)(rng);
    bs.Write(timestamp);
    bs.Write(RakNet::UNCONNECTED_MAGIC);
    peer->Send(&bs, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, RakNet::AddressOrGUID(ip, port), false);
}

void openConnectionRequest1(RakPeerInterface* peer, const char* ip, unsigned short port) {
    BitStream bs;
    bs.Write((MessageID)ID_OPEN_CONNECTION_REQUEST_1);
    bs.Write(RakNet::UNCONNECTED_MAGIC);
    uint64_t client_id = std::uniform_int_distribution<uint64_t>(1, UINT64_MAX)(rng);
    bs.Write(client_id);
    peer->Send(&bs, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, RakNet::AddressOrGUID(ip, port), false);
}

void openConnectionRequest2(RakPeerInterface* peer, const char* ip, unsigned short port) {
    BitStream bs;
    bs.Write((MessageID)ID_OPEN_CONNECTION_REQUEST_2);
    bs.Write(RakNet::UNCONNECTED_MAGIC);
    bs.Write(port);
    uint64_t client_id = std::uniform_int_distribution<uint64_t>(1, UINT64_MAX)(rng);
    bs.Write(client_id);
    bs.Write(true); // Do you have security enabled?
    peer->Send(&bs, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, RakNet::AddressOrGUID(ip, port), false);
}

void connectionRequest(RakPeerInterface* peer, SystemAddress serverAddress) {
    BitStream bs;
    bs.Write((MessageID)ID_CONNECTION_REQUEST);
    uint64_t client_id = std::uniform_int_distribution<uint64_t>(1, UINT64_MAX)(rng);
    bs.Write(client_id);
    peer->Send(&bs, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}

void sendAppAuthentication(RakPeerInterface* peer, SystemAddress serverAddress) {
    BitStream bs;
    bs.Write((MessageID)ID_USER_PACKET_ENUM + 1); // Custom authentication packet
    std::string username = "TestUser" + std::to_string(std::uniform_int_distribution<>(1000, 9999)(rng));
    std::string password = "P@ssw0rd" + std::to_string(std::uniform_int_distribution<>(1000, 9999)(rng));
    bs.Write(username.c_str());
    bs.Write(password.c_str());
    peer->Send(&bs, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}

void legitRakNetClient(const std::string& ip, unsigned short port, int duration) {
    RakPeerInterface* peer = RakPeerInterface::GetInstance();
    SocketDescriptor sd;
    peer->Startup(1, &sd, 1);

    sendUnconnectedPing(peer, ip.c_str(), port);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    openConnectionRequest1(peer, ip.c_str(), port);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    openConnectionRequest2(peer, ip.c_str(), port);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    Packet* packet = nullptr;
    SystemAddress serverAddress;
    auto start = std::chrono::steady_clock::now();
    bool connected = false;

    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < duration) {
        for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()) {
            if (packet->data[0] == ID_CONNECTION_ACCEPTED) {
                serverAddress = packet->systemAddress;
                connected = true;
                std::cout << "Connected to server!" << std::endl;
                connectionRequest(peer, serverAddress);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                sendAppAuthentication(peer, serverAddress);
            }
        }
        if (!connected) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            // Simula actividad legítima: pings, mensajes, etc.
            sendUnconnectedPing(peer, ip.c_str(), port);
            std::this_thread::sleep_for(std::chrono::milliseconds(std::uniform_int_distribution<>(500, 2000)(rng)));
        }
    }
    peer->Shutdown(300);
    RakPeerInterface::DestroyInstance(peer);
}

int main() {
    std::string ip;
    unsigned short port;
    int duration;

    std::cout << "Ingresa la IP del servidor: ";
    std::cin >> ip;
    std::cout << "Ingresa el puerto: ";
    std::cin >> port;
    std::cout << "Ingresa el tiempo de la prueba (segundos): ";
    std::cin >> duration;

    legitRakNetClient(ip, port, duration);

    std::cout << "Simulación finalizada." << std::endl;
    return 0;
}
