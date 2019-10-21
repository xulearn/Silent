#pragma once

// C++
#include <string>
#include <vector>
#include <ctime>

// ============== Network ==============
// Sockets and stuff
#include <winsock2.h>

// Adress translation
#include <ws2tcpip.h>

// Winsock 2 Library
#pragma comment(lib,"Ws2_32.lib")


class MainWindow;
class AudioService;



class NetworkService
{
public:

    NetworkService(MainWindow* pMainWindow, AudioService* pAudioService);

    // get
    std::string getClientVersion();
    std::string getUserName();

    void start(std::string adress, std::string port, std::string userName);
    void connectTo(std::string adress, std::string port, std::string userName);
    void setupVoiceConnection();

    void listenTCPFromServer();
    void listenUDPFromServer();

    void receiveInfoAboutNewUser();
    void receiveMessage();
    void deleteDisconnectedUserFromList();
    void receivePing();

    void sendMessage(std::wstring message);
    void sendVoiceMessage(char* pVoiceMessage, int iMessageSize, bool bLast);

    void disconnect();
    void lostConnection();
    void answerToFIN();
    void stop();

private:

    void serverMonitor();


    MainWindow* pMainWindow;
    AudioService* pAudioService;

    SOCKET userTCPSocket;
    SOCKET userUDPSocket;
    sockaddr_in serverAddr;

    std::string userName;

    bool bWinSockLaunched;
    bool bTextListen;
    bool bVoiceListen;

    clock_t lastTimeServerKeepAliveCame;

    std::string clientVersion;
};