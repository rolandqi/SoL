#ifndef HTTPDATA_H
#define HTTPDATA_H

#include <unordered_map>
#include <string>
#include <cstddef>
#include <queue>
#include <vector>
#include <pthread.h>

#include "utility.h"

using namespace std;

const int STATE_PARSE_URI = 1;
const int STATE_PARSE_HEADERS = 2;
const int STATE_RECV_BODY = 3;
const int STATE_ANALYSIS = 4;
const int STATE_FINISH = 5;

const int MAX_BUFF = 4096;

const int PARSE_URI_AGAIN = -1;
const int PARSE_URI_ERROR = -2;
const int PARSE_URI_SUCCESS = 0;

const int PARSE_HEADER_AGAIN = -1;
const int PARSE_HEADER_ERROR = -2;
const int PARSE_HEADER_SUCCESS = 0;

const int ANALYSIS_ERROR = -2;
const int ANALYSIS_SUCCESS = 0;

const int METHOD_POST = 1;
const int METHOD_GET = 2;
const int HTTP_10 = 1;
const int HTTP_11 = 2;
const int EPOLL_WAIT_TIME = 500;

enum HeadersState
{
    h_start = 0,
    h_key,
    h_colon,
    h_spaces_after_colon,
    h_value,
    h_CR,
    h_LF,
    h_end_CR,
    h_end_LF
};

class MimeType
{
private:
    static pthread_mutex_t mutexLock;
    static void init();
    MimeType();
    MimeType& operator=(const MimeType&);
    MimeType(const MimeType&);
    static unordered_map<string, string> mime;
public:
    static string getMime(const string &suffix); 
};

struct mytimer;

struct requestData
{
public:
    requestData();
    requestData(int _epollfd, int _fd, std::string _path);
    ~requestData();
    void addTimer(mytimer *mtimer);
    void reset();
    void seperateTimer();
    int getFd();
    void setFd(int _fd);
    void handleRequest();
    void handleError(int fd, int err_num, std::string short_msg);

private:
    std::string path;
    int fd;
    int epollfd;
    // content的内容用完就清空
    std::string content;
    int method;
    int HTTPversion;
    std::string file_name;
    int now_read_pos;
    int state;
    int h_state;
    bool isfinish;
    bool keep_alive;
    std::unordered_map<std::string, std::string> headers;
    mytimer *timer;
    static pthread_mutex_t requestLock;

    int parse_URI();
    int parse_Headers();
    int analysisRequest();
};

struct mytimer
{
private:
    bool deleted;
    size_t expired_time;
    requestData *request_data;  // 在timer里面delete掉request data

public:
    mytimer(requestData *request_data, int timeout);
    ~mytimer();
    void update(int timeout);
    bool isvalid();
    void clearReq();  // TODO 为什么这个地方没有delete掉request
    void setDeleted();
    bool isDeleted() const;
    size_t getExpTime() const;
};

struct comparator {
public:
    bool operator() (mytimer* a, mytimer* b) {   // a > b  从小到大
        return a->getExpTime() > b->getExpTime();
    }
};

priority_queue<mytimer*, vector<mytimer*>, comparator> myTimerQueue;

#endif