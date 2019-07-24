/*
 * Exception.h
 *
 *  Created on: Jul 22, 2019
 *      Author: kaiqi
 */

#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include "base/CurrentThread.h"

#include <exception>
#include <string>

class Exception : public std::exception  // not used.
{
 public:
    Exception(std::string msg)
      : message_(std::move(msg)),
        stack_(CurrentThread::stackTrace(/*demangle=*/false))
    {
    }
    ~Exception() noexcept override = default;

  // default copy-ctor and operator= are okay.

    const char* what() const noexcept override
    {
        return message_.c_str();
    }

    const char* stackTrace() const noexcept
    {
        return stack_.c_str();
    }

 private:
      std::string message_;
      std::string stack_;
};



#endif /* BASE_EXCEPTION_H_ */
