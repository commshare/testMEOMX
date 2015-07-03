/*------------------------------------------------------------------------------
    OSAL_MutexCreate
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_MutexCreate(OSAL_PTR *phMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)
                                OSAL_Malloc(sizeof(pthread_mutex_t));
    static pthread_mutexattr_t oAttr;
    static pthread_mutexattr_t *pAttr = NULL;

    if (pAttr == NULL &&
        !pthread_mutexattr_init(&oAttr) &&
        !pthread_mutexattr_settype(&oAttr, PTHREAD_MUTEX_RECURSIVE))
    {
        pAttr = &oAttr;
    }

    if (pMutex == NULL)
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;

    if (pthread_mutex_init(pMutex, pAttr)) {
        OSAL_Free(pMutex);
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;
    }

    *phMutex = (void *)pMutex;
    return OSAL_ERRORNONE;
}


/*------------------------------------------------------------------------------
    OSAL_MutexDestroy
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_MutexDestroy(OSAL_PTR hMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)hMutex;

    if (pMutex == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    if (pthread_mutex_destroy(pMutex)) {
        return OSAL_ERROR_BAD_PARAMETER;
    }

    OSAL_Free(pMutex);
	pMutex = NULL;

    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_MutexLock
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_MutexLock(OSAL_PTR hMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)hMutex;
    int err;

    if (pMutex == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    err = pthread_mutex_lock(pMutex);
    switch (err) {
    case 0:
        return OSAL_ERRORNONE;
    case EINVAL:
        return OSAL_ERROR_BAD_PARAMETER;
    case EDEADLK:
        return OSAL_ERROR_NOT_READY;
    default:
        return OSAL_ERROR_UNDEFINED;
    }

    return OSAL_ERRORNONE;
}

/*------------------------------------------------------------------------------
    OSAL_MutexUnlock
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_MutexUnlock(OSAL_PTR hMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)hMutex;
    int err;

    if (pMutex == NULL)
        return OSAL_ERROR_BAD_PARAMETER;

    err = pthread_mutex_unlock(pMutex);
    switch (err) {
    case 0:
        return OSAL_ERRORNONE;
    case EINVAL:
        return OSAL_ERROR_BAD_PARAMETER;
    case EPERM:
        return OSAL_ERROR_NOT_READY;
    default:
        return OSAL_ERROR_UNDEFINED;
    }

    return OSAL_ERRORNONE;
}
