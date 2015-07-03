
#include <pthread.h>

#include <signal.h>

typedef struct OSAL_THREADDATATYPE {
    pthread_t oPosixThread;
    pthread_attr_t oThreadAttr;
    OSAL_U32 (*pFunc)(OSAL_PTR pParam);
    OSAL_PTR pParam;
    OSAL_U32 uReturn;
} OSAL_THREADDATATYPE;



/*------------------------------------------------------------------------------
    BlockSIGIO      Linux EWL uses SIGIO to signal interrupt
------------------------------------------------------------------------------*/
static void BlockSIGIO()
{
    sigset_t set, oldset;

    /* Block SIGIO from the main thread to make sure that it will be handled
     * in the encoding thread */

    sigemptyset(&set);
    sigemptyset(&oldset);
    sigaddset(&set, SIGIO);
    pthread_sigmask(SIG_BLOCK, &set, &oldset);

}

/*------------------------------------------------------------------------------
    threadFunc
------------------------------------------------------------------------------*/
static void *threadFunc(void *pParameter)
{
    OSAL_THREADDATATYPE *pThreadData;
    pThreadData = (OSAL_THREADDATATYPE *)pParameter;
    pThreadData->uReturn = pThreadData->pFunc(pThreadData->pParam);
    return pThreadData;
}

/*------------------------------------------------------------------------------
    OSAL_ThreadCreate
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_ThreadCreate( OSAL_U32 (*pFunc)(OSAL_PTR pParam),
        OSAL_PTR pParam, OSAL_U32 nPriority, OSAL_PTR *phThread )
{
    OSAL_THREADDATATYPE *pThreadData;
    struct sched_param sched;

    pThreadData = (OSAL_THREADDATATYPE*)OSAL_Malloc(sizeof(OSAL_THREADDATATYPE));
    if (pThreadData == NULL)
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;

    pThreadData->pFunc = pFunc;
    pThreadData->pParam = pParam;
    pThreadData->uReturn = 0;

    pthread_attr_init(&pThreadData->oThreadAttr);

    pthread_attr_getschedparam(&pThreadData->oThreadAttr, &sched);
    sched.sched_priority += nPriority;
    pthread_attr_setschedparam(&pThreadData->oThreadAttr, &sched);

    if (pthread_create(&pThreadData->oPosixThread,
                       &pThreadData->oThreadAttr,
                       threadFunc,
                       pThreadData)) {
        OSAL_Free(pThreadData);
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;
    }

    BlockSIGIO();

    *phThread = (OSAL_PTR)pThreadData;
    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_ThreadDestroy
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_ThreadDestroy( OSAL_PTR hThread )
{
    OSAL_THREADDATATYPE *pThreadData = (OSAL_THREADDATATYPE *)hThread;
    void *retVal;

    if (pThreadData == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    //pthread_cancel(pThreadData->oPosixThread);

    if (pthread_join(pThreadData->oPosixThread, &retVal)) {
        return OSAL_ERROR_BAD_PARAMETER;
    }

    OSAL_Free(pThreadData);
    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_ThreadSleep
------------------------------------------------------------------------------*/
void OSAL_ThreadSleep(OSAL_U32 ms)
{
    usleep(ms*1000);
}

