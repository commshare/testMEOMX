#include <pthread.h>

#include <signal.h>



typedef struct {
    OSAL_BOOL       bSignaled;
    pthread_mutex_t mutex;
    int             fd[2];
} OSAL_THREAD_EVENT;


/*------------------------------------------------------------------------------
    OSAL_EventCreate
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_EventCreate(OSAL_PTR *phEvent)//传入的是指针的指针
{
    OSAL_THREAD_EVENT *pEvent = OSAL_Malloc(sizeof(OSAL_THREAD_EVENT));

    if (pEvent == NULL)
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;

    pEvent->bSignaled = 0;

    if (pipe(pEvent->fd) == -1)/*建立管道得到一对文件描述符 */
    {
        OSAL_Free(pEvent);
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;
    }

    if (pthread_mutex_init(&pEvent->mutex, NULL))
    {
        close(pEvent->fd[0]);
        close(pEvent->fd[1]);
        OSAL_Free(pEvent);
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;
    }

	//phEvent是指针的指针。通过这种方式，把分配了内存的指针传递到函数外
    *phEvent = (OSAL_PTR)pEvent;
    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_EventDestroy
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_EventDestroy(OSAL_PTR hEvent)
{
    OSAL_THREAD_EVENT *pEvent = (OSAL_THREAD_EVENT *)hEvent;
    if (pEvent == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    if (pthread_mutex_lock(&pEvent->mutex))
        return OSAL_ERROR_BAD_PARAMETER;

    int err = 0;
    err = close(pEvent->fd[0]); assert(err == 0);
    err = close(pEvent->fd[1]); assert(err == 0);

    pthread_mutex_unlock(&pEvent->mutex);
    pthread_mutex_destroy(&pEvent->mutex);

    OSAL_Free(pEvent);
    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_EventReset
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_EventReset(OSAL_PTR hEvent)
{
    OSAL_THREAD_EVENT *pEvent = (OSAL_THREAD_EVENT *)hEvent;
    if (pEvent == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    if (pthread_mutex_lock(&pEvent->mutex))
        return OSAL_ERROR_BAD_PARAMETER;

    if (pEvent->bSignaled)
    {
		/*	       #include <unistd.h>
       ssize_t read(int fd, void *buf, size_t count);
       read()  attempts to read up to count bytes from file descriptor fd into
       the buffer starting at buf.
		*/
        // empty the pipe
        char c = 1;
        int ret = read(pEvent->fd[0], &c, 1);
        if (ret == -1)
            return OSAL_ERROR_UNDEFINED;
        pEvent->bSignaled = 0;
    }

    pthread_mutex_unlock(&pEvent->mutex);
    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_GetTime
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_EventSet(OSAL_PTR hEvent)
{
    OSAL_THREAD_EVENT *pEvent = (OSAL_THREAD_EVENT *)hEvent;
    if (pEvent == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    if (pthread_mutex_lock(&pEvent->mutex))
        return OSAL_ERROR_BAD_PARAMETER;

    if (!pEvent->bSignaled)
    {
        char c = 1;
        int ret = write(pEvent->fd[1], &c, 1);
        if (ret == -1)//write失败
            return OSAL_ERROR_UNDEFINED;
        pEvent->bSignaled = 1;
    }

    pthread_mutex_unlock(&pEvent->mutex);
    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_EventWait
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_EventWait(OSAL_PTR hEvent, OSAL_U32 uMsec,
        OSAL_BOOL* pbTimedOut)
{
    OSAL_BOOL signaled = 0;
    return OSAL_EventWaitMultiple(&hEvent, &signaled, 1, uMsec, pbTimedOut);
}

/*------------------------------------------------------------------------------
    OSAL_EventWaitMultiple
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_EventWaitMultiple(OSAL_PTR* hEvents,/*hEvents可能指向一个个数组，叫做数组指针*/
        OSAL_BOOL* bSignaled, OSAL_U32 nCount, OSAL_U32 mSecs/*本函数wait超时时间*/,
        OSAL_BOOL* pbTimedOut)
{
    assert(hEvents);
    assert(bSignaled);

    fd_set read;
    FD_ZERO(&read);

    int max=0;
    unsigned i=0;
    for (i=0; i<nCount; ++i)//count是hEvents指针数组的大小
    {
        OSAL_THREAD_EVENT* pEvent = (OSAL_THREAD_EVENT*)(hEvents[i]);

        if (pEvent == NULL)
            return OSAL_ERROR_BAD_PARAMETER;

        int fd = pEvent->fd[0];
        if (fd > max)
            max = fd;//这是在根据fd确定max的值

        FD_SET(fd, &read);
    }

    if (mSecs == INFINITE_WAIT)
    {
        int ret = select(max+1, &read, NULL, NULL, NULL);
        if (ret == -1)
            return OSAL_ERROR_UNDEFINED;
    }
    else
    {
        struct timeval tv;
        memset(&tv, 0, sizeof(struct timeval));
        tv.tv_usec = mSecs * 1000;
        int ret = select(max+1, &read, NULL, NULL, &tv);
        if (ret == -1)
            return OSAL_ERROR_UNDEFINED;
        if (ret == 0)
        {
            *pbTimedOut =  1;
        }
    }

    for (i=0; i<nCount; ++i)
    {
        OSAL_THREAD_EVENT* pEvent = (OSAL_THREAD_EVENT*)hEvents[i];

        if (pEvent == NULL)
            return OSAL_ERROR_BAD_PARAMETER;

        int fd = pEvent->fd[0];
		//监控read是否在fd集合中
        if (FD_ISSET(fd, &read))
            bSignaled[i] = 1;
        else
            bSignaled[i] = 0;
    }

    return OSAL_ERRORNONE;
}

