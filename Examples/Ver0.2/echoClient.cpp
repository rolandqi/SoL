/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-18 10:08:08
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 16:59:06
 */
#include "net/EventLoop.h"
#include "net/Client.h"

const int PORT = 8421;
const int MAX_SIZE = 20;

struct echoContent
{
    char size;
    char body[MAX_SIZE];
};

void echoClientRead(int fd)
{
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
    delete buffer;
}

void echoClientConnection(int fd)
{
    cout << "Server connected!" <<endl;
    echoContent myContent;
    myContent.size = 6;
    memmove(&myContent.body, "HELLO", 6);
    // myContent.body = "HELLO";
    char size = 6;

    // int nwrite = writen(fd, &size, 1);
    // if (nwrite != 1)
    // {
    //     cout<< "write size failed! size : " << static_cast<int>(size) << endl;
    // }
    // char buffer[MAX_SIZE] = "HELLO";
    // nwrite = writen(fd, reinterpret_cast<char*>(&buffer), static_cast<int>(size));
    int nwrite = writen(fd, reinterpret_cast<char*>(&myContent), static_cast<int>(myContent.size) + 1);
    if (nwrite != size + 1)
    {
        cout<< "write failed!" << endl;
    }
}

int main() {
    EventLoop loop;
    Client myClient(&loop, 1, PORT);  // 1 worker.
    myClient.setConnectionCallback(bind(echoClientConnection, _1));
    myClient.setMessageCallback(bind(echoClientRead, _1));
    myClient.start();
    loop.loop();
}