/*
 * FileUtil.h
 *
 *  Created on: Aug 21, 2019
 *      Author: kaiqi
 */

#ifndef BASE_FILEUTIL_H_
#define BASE_FILEUTIL_H_
#include "noncopyable.h"
#include <string>

class AppendFile : noncopyable
{
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    // append 会向文件写
    void append(const char *logline, const size_t len);
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE* fp_;
    char buffer_[64*1024];
};




#endif /* BASE_FILEUTIL_H_ */
