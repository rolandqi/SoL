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
    // 超时的一定都是读请求，没有"被动"写。  //TODO 这写的是什么玩意！！！
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.ptr = (void*)this;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    if (timer != nullptr)
    {
        timer->clearReq();
        timer = nullptr;
    }
    close(fd);

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
    char buf[MAX_BUFF];
    bool isError = false;
    while (true)
    {
        // 这个地方之所以处理起来这么麻烦，是因为不知道发过来的data到底有多少字节
        int readNum = readn(fd, buf, MAX_BUFF);
        if(readNum < 0)
        {
            isError = true;
            break;
        }
        string reads(buf, buf + readNum);
        content += reads;  // 每次的read内容直接附到content后面

        if (state == STATE_PARSE_URI)
        {
            int flag = parse_URI();
            if (flag == PARSE_URI_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_URI_ERROR)
            {
                perror("parse URL error");
                isError = true;
                break;
            }
            
        }

    }
}

void requestData::handleError(int fd, int err_num, std::string short_msg)
{

}

int requestData::parse_URI()
{
    string &str = content;
    // 读到完整的请求行再开始解析请求
    if (pos < 0)
    int pos = str.find('\r', now_read_pos);
    {
        return PARSE_URI_AGAIN;
    }
    // 去掉请求行所占的空间，节省空间
    string request_line = str.substr(0, pos);
    if (str.size() > pos + 1)
        str = str.substr(pos + 1);
    else 
        str.clear();
    // Method
    pos = request_line.find("GET");
    if (pos < 0)
    {
        pos = request_line.find("POST");
        if (pos < 0)
        {
            return PARSE_URI_ERROR;
        }
        else
        {
            method = METHOD_POST;
        }
    }
    else
    {
        method = METHOD_GET;
    }
    //printf("method = %d\n", method);
    // filename
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        int _pos = request_line.find(' ', pos);
        if (_pos < 0)
            return PARSE_URI_ERROR;
        else
        {
            if (_pos - pos > 1)
            {
                file_name = request_line.substr(pos + 1, _pos - pos - 1);
                int __pos = file_name.find('?');
                if (__pos >= 0)
                {
                    file_name = file_name.substr(0, __pos);
                }
            }
                
            else
                file_name = "index.html";
        }
        pos = _pos;
    }
    //cout << "file_name: " << file_name << endl;
    // HTTP 版本号
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        if (request_line.size() - pos <= 3)
        {
            return PARSE_URI_ERROR;
        }
        else
        {
            string ver = request_line.substr(pos + 1, 3);
            if (ver == "1.0")
                HTTPversion = HTTP_10;
            else if (ver == "1.1")
                HTTPversion = HTTP_11;
            else
                return PARSE_URI_ERROR;
        }
    }
    state = STATE_PARSE_HEADERS;
    return PARSE_URI_SUCCESS;
}

int requestData::parse_Headers()
{
    string &str = content;
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    for (int i = 0; i < str.size() && notFinish; ++i)
    {
        switch(h_state)
        {
            case h_start:
            {
                if (str[i] == '\n' || str[i] == '\r')
                    break;
                h_state = h_key;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
            case h_key:
            {
                if (str[i] == ':')
                {
                    key_end = i;
                    if (key_end - key_start <= 0)
                        return PARSE_HEADER_ERROR;
                    h_state = h_colon;
                }
                else if (str[i] == '\n' || str[i] == '\r')
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case h_colon:
            {
                if (str[i] == ' ')
                {
                    h_state = h_spaces_after_colon;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case h_spaces_after_colon:
            {
                h_state = h_value;
                value_start = i;
                break;  
            }
            case h_value:
            {
                if (str[i] == '\r')
                {
                    h_state = h_CR;
                    value_end = i;
                    if (value_end - value_start <= 0)
                        return PARSE_HEADER_ERROR;
                }
                else if (i - value_start > 255)
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case h_CR:
            {
                if (str[i] == '\n')
                {
                    h_state = h_LF;
                    string key(str.begin() + key_start, str.begin() + key_end);
                    string value(str.begin() + value_start, str.begin() + value_end);
                    headers[key] = value;
                    now_read_line_begin = i;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case h_LF:
            {
                if (str[i] == '\r')
                {
                    h_state = h_end_CR;
                }
                else
                {
                    key_start = i;
                    h_state = h_key;
                }
                break;
            }
            case h_end_CR:
            {
                if (str[i] == '\n')
                {
                    h_state = h_end_LF;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_end_LF:
            {
                notFinish = false;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
        }
    }
    if (h_state == h_end_LF)
    {
        str = str.substr(now_read_line_begin);
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
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