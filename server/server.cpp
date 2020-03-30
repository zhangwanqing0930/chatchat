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

//string转换函数
template<typename T> string toString(const T& t) {
    stringstream ss{};     //创建一个格式化输出流
    ss << t;               //把值传递如流中
    return ss.str();
}

int main()
{
    //链接ws2_32.lib，创建套接字，这是接下来绑定socket的必要步骤
    WSADATA wsaData;
    if (WSAStartup(WINSOCKVERSION, &wsaData)) {
        cout << "ERROR: WSAStartup can not open" << endl;
        return 1;
    }

    //进一步绑定套接字，初始化参数
    SOCKET serSocket = socket(AF_INET, SOCK_STREAM, 0); //创建了可识别套接字，TCP/IP IPv4
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      //ip地址，这里是本地地址
    addr.sin_port = htons(38888);                       //绑定端口

    //绑定参数
    if (bind(serSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR))) {
        cout << "ERROR: bind error" << endl;
        return 1;
    }
    //开始监听，其中第二个参数代表能够接收的最多的连接数
    if (listen(serSocket, 5)) {
        cout << "ERROR: listen error" << endl;
        return 1;
    };

    //定义其他参数
    vector<SOCKET> client_user;            //保存已经链接的客户端socket            
    char receiveBuf[200];                  //接收buf
    string sendBuf;                        //发送buf

    //用select方式循环监听
    while (1) {

        //初始化所有待监听的socket
        fd_set fdRead;
        FD_ZERO(&fdRead);                   //清空fdRead，保证里面为空
        FD_SET(serSocket, &fdRead);         //将serSocket放入fdRead，serSocket用来监听连接请求
        for (auto item : client_user) {     //将所有已经链接的客户端socket放入fdRead，用来接收发送的消息
            FD_SET(item, &fdRead);
        }

        //select阻塞检测所有socket是否被激活，当有一个socket激活时，函数执行完毕，fdRead里会删除没有被激活的socket
        cout << "waiting for connection" << endl;
        int ret = select(0, &fdRead, nullptr, nullptr, nullptr);

        //如果serSocket被激活，是链接请求，初始化一个客户端socket，存入client_user
        if (FD_ISSET(serSocket, &fdRead)) {
            FD_CLR(serSocket, &fdRead);     //在fdRead里删除serSocket,以免它进入接收信息的过程中
            SOCKADDR_IN clientsocket;
            SOCKET serConn = INVALID_SOCKET;
            int len = sizeof(SOCKADDR_IN);
            if (serConn = accept(serSocket, (SOCKADDR*)&clientsocket, &len)) {
                client_user.push_back(serConn);
                cout << "got connection: " << serConn << endl;
            }
        }

        //如果fdRead里面有激活的socket，并且不是serSocket，接收信息并广播信息
        for (int i = 0; i < fdRead.fd_count; i++) {
            auto client_id = fdRead.fd_array[i];
            //如果用户断线，将其从列表中剔出
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
            //否则接收信息并广播信息
            else {
                cout << "recv msg: " << receiveBuf << " from: " << client_id << endl;
                for (auto client : client_user) {
                    if (client != client_id) {
                        sendBuf = "用户 " + toString<SOCKET>(client_id) + ": " + receiveBuf;
                        send(client, sendBuf.c_str(), 200, 0);
                        cout << "send msg: " << sendBuf << " to: " << client << endl;
                    }
                }
            }
        }
    }

    //关闭
    for (auto client : client_user) {
        closesocket(client);
    }
    WSACleanup();
    return 0;
}
