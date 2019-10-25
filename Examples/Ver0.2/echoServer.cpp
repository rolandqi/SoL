/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:01:25
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 16:58:56
 */
#include "net/EventLoop.h"
#include "net/Server.h"

const int PORT = 8421;
const int MAX_SIZE = 20;
using std::placeholders::_1;
using std::placeholders::_2;

struct echoContent
{
    char size;
    char body[MAX_SIZE];
};


void echoServerRead(const int& fd)
{
    // receive data: 1 byte length + data.
    char size = 0;
    int nread = readn(fd, &size, 1);  // read out the lenth of the data.
    if (nread != 1)
    {
        cout << "Receiving data error in fd: " << fd << endl;
    }
    char* buffer = new char[size];
    memset(buffer, 0, size);
    nread = readn(fd, buffer, size);
    if (size > 0)
    {
        cout<< "receiving data size: "<< static_cast<int>(size) << endl;
        cout << "Receiving data :" << buffer << endl;
    }
    if (nread != size)
    {
        cout << "Receiving data error in fd: " << fd << endl;
    }

    // echo back
    echoContent myContent;
    myContent.size = size;
    memmove(&myContent.body, buffer, size);
    delete buffer;
    // int nwrite = writen(fd, &size, 1);
    // if (nwrite != 1)
    // {
    //     cout<< "write size failed! size : " << static_cast<int>(size) << endl;
    // }
    int nwrite = writen(fd, reinterpret_cast<char*>(&myContent), static_cast<int>(size) + 1);
    if (nwrite != size + 1)
    {
        cout<< "write failed! write size: " << nwrite << endl;
    }
}

void echoServerConnection(const sockaddr_in& request)
{
    cout << "Connect one client!" << endl;
}

int main() {
    EventLoop loop;
    Server myServer(&loop, 1, PORT);
    myServer.setConnectionCallback(bind(echoServerConnection, _1));
    myServer.setMessageCallback(bind(echoServerRead, _1));
    myServer.start();
    loop.loop();

}