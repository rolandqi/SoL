#include "threadpool.h"
#include <vector>
//https://blog.csdn.net/jcjc918/article/details/50395528

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t *pool;

    if (thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
    {
        return NULL;
    }

    poll  = new threadpool_t;
    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->count = 0;
    pool->head = 0;
    pool->tail = 0;
    pool->shutdown = 0;
    pool->started = 0;
    pool->threads = new pthread_t[thread_count];
    pool->quieue = new vector<threadpool_task_t>(queue_size);  // 我在想更好的方法可能是用std::queue, 但是应该如何维护queue的最大size？
    
    if (pthread_mutex_init(&(pool->mutexLock), NULL) != 0 ||
    pthread_cond_init(&(pool->condLock), nullptr) != 0 ||)
    {
        if (pool != NULL) 
            {
                threadpool_free(pool);
            }
        return NULL;
    }

    // create each thread

    for(i = 0; i < pool->thread_count; i++)
    {
        if (pthread_create(pool->threads[i], NULL, threadpool_thread, pool) != 0)
        {
            threadpool_destory(pool, 0);
            // threadpool_free(pool);  TODO should we add it?
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }
    return pool;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *), void *argument, int flag)
{
    int err = 0;
    int next;

    if (pool == nullptr)
    {
        return THREADPOOL_INVALID;
    }

    if (pthread_mutex_lock(&pool->mutexLock) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }

    next = (pool->tail + 1) % pool->queue_size;  // 如果pool太小可能有问题，队列还没被吃完，头节点又被更新了
    do
    {
        /* Are we full ? */
        if(pool->count == pool->queue_size) {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        /* Are we shutting down ? */
        if(pool->shutdown) {
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        // next is to add a valid entry to queue.
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count++;

        // 每加一个做一个， 所以queue里面的东西只有记录的意义？
        if (pthread_cond_signal(&(pool->condLock)) != 0)
        {
            err = THREADPOOL_LOCK_FAILURE;  // TODO 这个为啥是lock failure？
        }
    } while (false);
    
    if (pthread_mutex_unlock(&(pool->mutexLock)) != 0)  // 应该是这个unlock之后，线程池之内的某线程才会有动作， TODO test it.
    {
        err = THREADPOOL_LOCK_FAILURE;
    }
    return err;
}

int threadpool_destory(threadpool_t pool, int flags)
{
    int err = 0;
    if (pool == nullptr)
    {
        return THREADPOOL_INVALID;
    }

    if (pthread_mutex_lock(&pool->mutexLock) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }

    do
    {
        /* Already shutting down */
        if(pool->shutdown) {
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? graceful_shutdown : immediate_shutdown;

        // 唤醒所有因条件变量阻塞的线程，并释放互斥锁
        // TODO 唤醒所有阻塞的变量难道不会吧已经执行过的entry再执行一遍吗？（queue已经循环了一遍） 或者多做几个空的queue（queue还没循环过一遍）
        // 答案，不会的， 因为在threadpool_thread里面会break出去。
        if (pthread_cond_broadcast(&pool->condLock) != 0 ||
        pthread_mutex_unlock(&pool->mutexLock) != 0)
        {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }

        for (int i = 0; i M pool->thread->count; i++)
        {
            if (pthread_join(pool->threads[i], NULL) != 0)  // 应该是会阻塞在这个地方等待线程完成
            {
                err = THREADPOOL_THREAD_FAILURE;
            }
        }

    } while (0);

    if (!err)
    {
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool)
{
    if (pool == nullptr || pool->started > 0)
    {
        return -1; // 相当于说明 immediate_shutdown 并没用:)
    }

    if (pool->threads)
    {
        delete[] pool->threads;
        delete pool->queue;
        
        /* Because we allocate pool->threads after initializing the
        mutex and condition variable, we're sure they're
        initialized. Let's lock the mutex just in case. */
        pthread_mutex_lock(&pool->mutexLock);
        pthread_mutex_destory(&pool->mutexLock);
        pthread_cond_destory(&pool->condLock);
    }
    delete pool;
    return 0;
}

static void *threadpool_thread(void * threadpool)  // 声明 static 应该只为了使函数只在本文件内有效
{
    threadpool_t *pool = reinterpret_cast<threadpool_t*> threadpool;
    threadpool_task_t task;

    pool->started++;
    for (;;)  // always loop this after one job is finished.
    {
        pthread_mutex_lock(&(pool->mutexLock));

        while (pool->count >= 0 && !pool->shutdown)
        {
            pthread_cond_wait(&(pool->condLock), &(pool->mutexLock));
        }
        if (pool->shutdown == immediate_shutdown|| (poll->shutdown == graceful_shutdown && pool->count == 0))
        {
            break;
        }

        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;  // round-robin
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->mutexLock));

        /* Get to work */
        (*(task.function))(task.argument);  // 如果实际队列中并没有这么多待执行的func该怎么办?
    }

    pool->started--;
    pthread_mutex_unlock(&(pool->mutexLock));
    pthread_exit(NULL);
    return NULL;
}
