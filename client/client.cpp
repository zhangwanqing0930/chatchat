#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#pragma comment(lib,"ws2_32.lib")

#define WINSOCKVERSION MAKEWORD(2, 2)

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::thread;

//�����첽��Ϣ����
void recvFunc(SOCKET& clientSocket) {
    while (true) {
        char receiveBuf[200] = "\0";
        if (recv(clientSocket, receiveBuf, sizeof(receiveBuf), 0) < 0) {
            cout << "����������" << endl;


            return;
        }
        cout << "�յ�: " << receiveBuf << '\n' << endl;
    }
}


int main() {

    //����ws2_32.lib�������׽��֣����ǽ�������socket�ı�Ҫ����
    WSADATA wsaData;
    if (WSAStartup(WINSOCKVERSION, &wsaData)) {
        cout << "ERROR: WSAStartup can not open" << endl;
        return 1;
    }

    //��һ�����׽��֣���ʼ������
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN clientsock_in;
    string ip = "127.0.0.1";
    inet_pton(AF_INET, ip.data(), (void*)&clientsock_in.sin_addr.s_addr);   //ip�趨
    clientsock_in.sin_family = AF_INET;                                     //Ipv4
    clientsock_in.sin_port = htons(38888);                                  //�˿ں�

    //�ͻ�������server
    if (connect(clientSocket, (SOCKADDR*)&clientsock_in, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        cout << "����ʧ�ܣ�" << endl;
        return 1;
    };

    cout << "������䰴�»س����ɷ�����Ϣ��" << endl;

    //���ӳɹ���������������Ϣ���߳�
    thread handler(recvFunc, std::ref(clientSocket));
    handler.detach();

    //���ϸ�server������Ϣ
    while (true) {
        char sendBuf[100];
        cin >> sendBuf;
        cout << "���ͳɹ�\n" << endl;
        send(clientSocket, sendBuf, strlen(sendBuf) + 1, 0);
    }
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}