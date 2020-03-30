#include <iostream>
#include <vector>
#include <winsock2.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstdlib>
#pragma comment(lib,"ws2_32.lib")

#define WINSOCKVERSION MAKEWORD(2, 2)

using std::endl;
using std::cout;
using std::vector;
using std::string;
using std::stringstream;

//stringת������
template<typename T> string toString(const T& t) {
    stringstream ss{};     //����һ����ʽ�������
    ss << t;               //��ֵ����������
    return ss.str();
}

int main()
{
    //����ws2_32.lib�������׽��֣����ǽ�������socket�ı�Ҫ����
    WSADATA wsaData;
    if (WSAStartup(WINSOCKVERSION, &wsaData)) {
        cout << "ERROR: WSAStartup can not open" << endl;
        return 1;
    }

    //��һ�����׽��֣���ʼ������
    SOCKET serSocket = socket(AF_INET, SOCK_STREAM, 0); //�����˿�ʶ���׽��֣�TCP/IP IPv4
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      //ip��ַ�������Ǳ��ص�ַ
    addr.sin_port = htons(38888);                       //�󶨶˿�

    //�󶨲���
    if (bind(serSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR))) {
        cout << "ERROR: bind error" << endl;
        return 1;
    }
    //��ʼ���������еڶ������������ܹ����յ�����������
    if (listen(serSocket, 5)) {
        cout << "ERROR: listen error" << endl;
        return 1;
    };

    //������������
    vector<SOCKET> client_user;            //�����Ѿ����ӵĿͻ���socket            
    char receiveBuf[200];                  //����buf
    string sendBuf;                        //����buf

    //��select��ʽѭ������
    while (1) {

        //��ʼ�����д�������socket
        fd_set fdRead;
        FD_ZERO(&fdRead);                   //���fdRead����֤����Ϊ��
        FD_SET(serSocket, &fdRead);         //��serSocket����fdRead��serSocket����������������
        for (auto item : client_user) {     //�������Ѿ����ӵĿͻ���socket����fdRead���������շ��͵���Ϣ
            FD_SET(item, &fdRead);
        }

        //select�����������socket�Ƿ񱻼������һ��socket����ʱ������ִ����ϣ�fdRead���ɾ��û�б������socket
        cout << "waiting for connection" << endl;
        int ret = select(0, &fdRead, nullptr, nullptr, nullptr);

        //���serSocket��������������󣬳�ʼ��һ���ͻ���socket������client_user
        if (FD_ISSET(serSocket, &fdRead)) {
            FD_CLR(serSocket, &fdRead);     //��fdRead��ɾ��serSocket,���������������Ϣ�Ĺ�����
            SOCKADDR_IN clientsocket;
            SOCKET serConn = INVALID_SOCKET;
            int len = sizeof(SOCKADDR_IN);
            if (serConn = accept(serSocket, (SOCKADDR*)&clientsocket, &len)) {
                client_user.push_back(serConn);
                cout << "got connection: " << serConn << endl;
            }
        }

        //���fdRead�����м����socket�����Ҳ���serSocket��������Ϣ���㲥��Ϣ
        for (int i = 0; i < fdRead.fd_count; i++) {
            auto client_id = fdRead.fd_array[i];
            //����û����ߣ�������б����޳�
            if (recv(client_id, receiveBuf, sizeof(receiveBuf), 0) < 0) {
                cout << client_id << " disconnected" << endl;
                for (auto client = client_user.begin(); client != client_user.end();) {
                    if (*client == client_id) {
                        client = client_user.erase(client);
                        closesocket(client_id);
                    }
                    else {
                        client++;
                    }
                }
            }
            //���������Ϣ���㲥��Ϣ
            else {
                cout << "recv msg: " << receiveBuf << " from: " << client_id << endl;
                for (auto client : client_user) {
                    if (client != client_id) {
                        sendBuf = "�û� " + toString<SOCKET>(client_id) + ": " + receiveBuf;
                        send(client, sendBuf.c_str(), 200, 0);
                        cout << "send msg: " << sendBuf << " to: " << client << endl;
                    }
                }
            }
        }
    }

    //�ر�
    for (auto client : client_user) {
        closesocket(client);
    }
    WSACleanup();
    return 0;
}
