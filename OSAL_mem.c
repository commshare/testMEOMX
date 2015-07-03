#include "OSAL.h"

/*------------------------------------------------------------------------------
    Definitions
------------------------------------------------------------------------------*/

#define MEMORY_SENTINEL 0xACDCACDC;

/*------------------------------------------------------------------------------
    External compiler flags
--------------------------------------------------------------------------------

MEMALLOCHW: If defined we will use kernel driver for HW memory allocation


--------------------------------------------------------------------------------
    OSAL_Malloc
------------------------------------------------------------------------------*/
OSAL_PTR OSAL_Malloc( OSAL_U32 size )
{
    OSAL_U32 extra = sizeof(OSAL_U32) * 2;
    OSAL_U8*  data = (OSAL_U8*)malloc(size + extra);
    OSAL_U32 sentinel = MEMORY_SENTINEL;

    memcpy(data, &size, sizeof(size));
    memcpy(&data[size + sizeof(size)], &sentinel, sizeof(sentinel));

    return data + sizeof(size);
}

/*------------------------------------------------------------------------------
    OSAL_Free
------------------------------------------------------------------------------*/
void OSAL_Free( OSAL_PTR pData )
{
    OSAL_U8* block    = ((OSAL_U8*)pData) - sizeof(OSAL_U32);
    OSAL_U32 sentinel = MEMORY_SENTINEL;
    OSAL_U32 size     = *((OSAL_U32*)block);

    assert(memcmp(&block[size+sizeof(size)],&sentinel, sizeof(sentinel))==0 &&
            "mem corruption detected");

    free(block);
}

/*------------------------------------------------------------------------------
    OSAL_Memset
------------------------------------------------------------------------------*/
OSAL_PTR OSAL_Memset( OSAL_PTR pDest, OSAL_U32 cChar, OSAL_U32 nCount)
{
    return memset(pDest, cChar, nCount);
}

/*------------------------------------------------------------------------------
    OSAL_Memcpy
------------------------------------------------------------------------------*/
OSAL_PTR OSAL_Memcpy( OSAL_PTR pDest, OSAL_PTR pSrc, OSAL_U32 nCount)
{
    return memcpy(pDest, pSrc, nCount);
}
