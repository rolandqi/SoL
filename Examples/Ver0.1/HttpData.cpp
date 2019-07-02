#include "HttpData.h"
#include "epoll.h"
#include <sys/time.h>
#include <iostream>
#include <stddef.h>  // for NULL

pthread_mutex_t MimeType::mutexLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t requestData::requestLock = PTHREAD_MUTEX_INITIALIZER;

void MimeType::init()
{
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "application/x-ico";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}

string MimeType::getMime(const string &suffix)                // TODO 这个string是不是不需要加啊？
{
    if (mime.empty())
    {
        pthread_mutex_lock(&mutexLock);
        if (mime.size() == 0)
        {
            init();
        }
        pthread_mutex_unlock(&mutexLock);
    }
    if (mime.find(suffix) == mime.end())
    {
        return mime["default"];
    }
    else
    {
        return mime.at(suffix);
    }
}

requestData::requestData() :
                now_read_pos(0),
                state(STATE_PARSE_URI),
                h_state(h_start),
                keep_alive(false),
                againTimes(0),
                timer(nullptr),
                path(),  // TODO 用这种方式命令空串可行吗？ 
                fd(0), 
                epollfd(0)
{
    cout<<"request data constructor."<<endl;
}

requestData::requestData(int _epollfd, int _fd, std::string _path) :
                now_read_pos(0), 
                state(STATE_PARSE_URI), 
                h_state(h_start),
                keep_alive(false), 
                againTimes(0), 
                timer(NULL),
                path(_path), 
                fd(_fd), 
                epollfd(_epollfd)
{
    cout<<"request data constructor with parameter."<<endl;
}

requestData::~requestData()
{
    cout<<"~requestData()"<<endl;
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.ptr = (void*)this;

}

void requestData::addTimer(mytimer *mtimer)
{
    if (timer == NULL)
    {
        timer = mtimer;
    }
    else
    {
        cout<<"timer already exist"<<endl;
    }
    
}

void requestData::reset()
{
    againTimes = 0;
    content.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
    h_state = h_start;
    headers.clear();
    keep_alive = false;
}

void requestData::seperateTimer()
{
    if (timer)
    {
        timer->clearReq();
        timer = NULL;
    }
}

int requestData::getFd()
{
    return fd;
}

void requestData::setFd(int _fd)
{
    this->fd = _fd;
}

void requestData::handleRequest()
{
    
}

void requestData::handleError(int fd, int err_num, std::string short_msg)
{

}

int requestData::parse_URI()
{

}

int requestData::parse_Headers()
{

}

int requestData::analysisRequest()
{

}


mytimer::mytimer(requestData *request_data, int timeout) : deleted(false), request_data(request_data)
{
    cout<<"mytimer init\n"<<endl;
    timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;  // uisng ms as counting unit.
}

mytimer::~mytimer()
{
    cout<<"~mytimer()"<<endl;
    if (request_data != nullptr)
    {
        cout<<"deleting request_data="<<request_data<<"\n"<<endl;
        delete request_data;
        request_data = nullptr;
    }
}

void mytimer::update(int timeout)
{
    struct timeval current;
    gettimeofday(&current, nullptr);
    expired_time = ((current.tv_sec * 1000) + (current.tv_usec / 1000)) + timeout;
}

bool mytimer::isvalid()
{
    struct timeval current;
    gettimeofday(&current, nullptr);
    size_t currentTimeinUs = ((current.tv_sec * 1000) + (current.tv_usec / 1000));
    if (currentTimeinUs < expired_time)
    {
        return true;
    }
    else
    {
        setDeleted();  // 这个时候不析构request_data 对象，而是等到析构mytimer的时候析构。
        return false;
    }
}

void mytimer::clearReq()
{
    request_data = nullptr;
    setDeleted();
}

void mytimer::setDeleted()
{
    deleted = true;
}

bool mytimer::isDeleted() const
{
    return deleted;
}

size_t mytimer::getExpTime() const
{
    return expired_time;
}