
#include <assert.h>
#include <string.h>

typedef BUFFER** swap_type;
static  /*ÕâÊÇÈı¸öÖ¸Õë°¡*/
	//Á©²ÎÊı¶¼ÊÇÖ¸Õë£¬ÔÚÊ¹ÓÃÖĞ£¬´«ÈëµÄÊÇµØÖ·¡£
void swap_ptr(swap_type* one, swap_type* two)
{
    swap_type temp = *one;//È¡µØÖ·²Ù×÷£¬ÕâÊÇ°ÑÖ¸ÕëÖ¸ÏòµÄÄÚÈİÈ¡³öÀ´
    *one = *two;
    *two = temp; //Ö¸ÕëtwoÖ¸ÏòµÄÄÚÈİ£¬´æ·ÅÁËÒ»¸öĞÂµÄÖµ¡£
}

OMX_ERRORTYPE SCOmx_bufferlist_init(BUFFERLIST* list, OMX_U32 size)
{ assert(list);

	/*
	BUFFER** Ã¿¸öÔªËØÊÇÒ»¸öBUFFER* ÀàĞÍÖ¸Õë¡£ÎªBUFFER**·ÖÅäsize¸ö´óĞ¡ÎªBUFFER*ÀàĞÍ´óĞ¡
	µÄ¿Õ¼ä¡£
	*/
    list->list = (BUFFER**)OSAL_Malloc(sizeof(BUFFER*) * size);
    if (!list->list)
        return OMX_ErrorInsufficientResources;

    memset(list->list, 0, sizeof(BUFFER*) * size);
    list->size     = 0;
    list->capacity = size;//³õÊ¼×´Ì¬ÏÂ£¬¿ÉÓÃµÈÓÚ×î´ó¡£
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SCOmx_bufferlist_reserve(BUFFERLIST* list, OMX_U32 newsize)
{
    assert(list);
    if (newsize < list->capacity)//ĞÂ´óĞ¡±ØĞë´óÓÚµÈÓÚ¿ÉÓÃ
        return OMX_ErrorBadParameter;

    BUFFER** data = (BUFFER**)OSAL_Malloc(sizeof(BUFFER**) * newsize);
    if (!data)
        return OMX_ErrorInsufficientResources;

    memset(data, 0, sizeof(BUFFER*) * newsize);
    memcpy(data, list->list, list->size * sizeof(BUFFER*));

	//´«ÈëµÄÊÇÁ©Ö¸ÕëµÄµØÖ·£¬¾­¹ıÕâ¸öº¯Êı£¬°ÑµØÖ·Ö¸ÏòµÄÄÚÈİ½»»»ÁË¡£
	//×¨Òµlist->listÏÖÔÚÖ¸ÏòÁËÒ»¸öĞÂµÄµØÖ·¡£
    swap_ptr(&data, &list->list);

    list->capacity = newsize;
	//Ö®Ç°ÄÇ¶Î¿Õ¼ä¾Í¿ÉÒÔÊÍ·ÅÁË¡£
    OSAL_Free(data);
    return OMX_ErrorNone;
}

void SCOmx_bufferlist_destroy(BUFFERLIST* list)
{
    assert(list);
    if (list->list)
        OSAL_Free(list->list);
	//listÖ¸ÕëÖ¸ÏòµÄÄÚ´æ¿Õ¼ä±»ÖØÖÃÎª0£¬²»freeÈ»ºóÖÃÖ¸ÕëÎªNULLÃ´å
    memset(list, 0, sizeof(BUFFERLIST));
}

OMX_U32 SCOmx_bufferlist_get_size(BUFFERLIST* list)
{
    assert(list);
    return list->size;
}

OMX_U32 SCOmx_bufferlist_get_capacity(BUFFERLIST* list)
{
    assert(list);
    return list->capacity;
}

BUFFER** SCOmx_bufferlist_at(BUFFERLIST* list, OMX_U32 i)
{
    assert(list);
    assert(i < list->size);
	/*
	×¢Òâ£¬ÎªÉ¶ÕâÀïÒª·µ»ØµÄÊÇµØÖ·¡
	ÒòÎª£¬list->listÊÇBUFFER* ÀàĞÍÖ¸Õë£¬list->list[i]µÃµ½µÄ¾ÍÊÇBUFFER* £¬ÒªÇó·µ»ØµÄÊÇBUFFER **,
	Ò²¾ÍÊÇBUFFER* ÀàĞÍµÄµØÖ·ÁË¡£
	¿´ÆğÀ´£¬Õâ¸ölist->listÊÇ¸öÖ¸ÕëÊı×é°¡¡£
	*/
    return &list->list[i];
}

void SCOmx_bufferlist_remove(BUFFERLIST* list, OMX_U32 i)
{
    assert(list);
    assert(i < list->size);
	/*Êı×éËäÈ»¿ÉÒÔËæ»úÉ¾³ı
	µ«ÊÇ¿ÕµÄÎ»ÖÃÒªÍ¨¹ımemoveÀ´²¹*/
	/*
	       #include <string.h>

       void *memmove(void *dest, const void *src, size_t n);

  The  memmove()  function  copies n bytes from memory area src to memory
       area dest.  The memory areas may overlap: copying takes place as though
       the  bytes in src are first copied into a temporary array that does not
       overlap src or dest, and the bytes are then copied from  the  temporary
       array to dest.

	*/
	//Èç¹ûiÊÇ´Ó0¿ªÊ¼ËãµÄ£¬ÄÇÃ´iÓÒ±ßÊ£ÓàµÄÔªËØÊıÄ¿¾ÍÊÇsize -i -1
    memmove(list->list+i, list->list+i+1, (list->size-i-1)*sizeof(BUFFER*));
    --list->size;
}

//Õâ¸ö²Ù×÷ÎªÉ¶Ö»ÓĞsize ÖÃ 0°¡
void SCOmx_bufferlist_clear(BUFFERLIST* list)
{
    assert(list);
    list->size = 0;
}

OMX_BOOL SCOmx_bufferlist_push_back(BUFFERLIST* list, BUFFER* buff)
{
    assert(list);
    if (list->size == list->capacity)//³õÊ¼»¯¾ÍÊÇÕâÑùµÄ°¡£¬ÓĞÉ¶´íÎóå
        return OMX_FALSE;
    list->list[list->size++] = buff;
    return OMX_TRUE;
}
