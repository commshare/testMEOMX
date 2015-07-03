#include <sys/time.h>


/*------------------------------------------------------------------------------
    OSAL_GetTime
------------------------------------------------------------------------------*/
OSAL_U32 OSAL_GetTime()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((OSAL_U32)now.tv_sec) * 1000 + ((OSAL_U32)now.tv_usec) / 1000;
}


