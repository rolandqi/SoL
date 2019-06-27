/*
 * noncopyable.h
 *
 *  Created on: Jun 27, 2019
 *      Author: kaiqi
 */

#ifndef BASE_NONCOPYABLE_H_
#define BASE_NONCOPYABLE_H_

namespace sol
{

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
}



#endif /* BASE_NONCOPYABLE_H_ */
