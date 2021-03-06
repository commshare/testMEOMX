
#include <assert.h>
#include <string.h>

typedef BUFFER** swap_type;
static  /*这是三个指针啊*/
	//俩参数都是指针，在使用中，传入的是地址。
void swap_ptr(swap_type* one, swap_type* two)
{
    swap_type temp = *one;//取地址操作，这是把指针指向的内容取出来
    *one = *two;
    *two = temp; //指针two指向的内容，存放了一个新的值。
}

OMX_ERRORTYPE SCOmx_bufferlist_init(BUFFERLIST* list, OMX_U32 size)
{ assert(list);

	/*
	BUFFER** 每个元素是一个BUFFER* 类型指针。为BUFFER**分配size个大小为BUFFER*类型大小
	的空间。
	*/
    list->list = (BUFFER**)OSAL_Malloc(sizeof(BUFFER*) * size);
    if (!list->list)
        return OMX_ErrorInsufficientResources;

    memset(list->list, 0, sizeof(BUFFER*) * size);
    list->size     = 0;
    list->capacity = size;//初始状态下，可用等于最大。
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SCOmx_bufferlist_reserve(BUFFERLIST* list, OMX_U32 newsize)
{
    assert(list);
    if (newsize < list->capacity)//新大小必须大于等于可用
        return OMX_ErrorBadParameter;

    BUFFER** data = (BUFFER**)OSAL_Malloc(sizeof(BUFFER**) * newsize);
    if (!data)
        return OMX_ErrorInsufficientResources;

    memset(data, 0, sizeof(BUFFER*) * newsize);
    memcpy(data, list->list, list->size * sizeof(BUFFER*));

	//传入的是俩指针的地址，经过这个函数，把地址指向的内容交换了。
	//专业list->list现在指向了一个新的地址。
    swap_ptr(&data, &list->list);

    list->capacity = newsize;
	//之前那段空间就可以释放了。
    OSAL_Free(data);
    return OMX_ErrorNone;
}

void SCOmx_bufferlist_destroy(BUFFERLIST* list)
{
    assert(list);
    if (list->list)
        OSAL_Free(list->list);
	//list指针指向的内存空间被重置为0，不free然后置指针为NULL么�
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
	注意，为啥这里要返回的是地址�
	因为，list->list是BUFFER* 类型指针，list->list[i]得到的就是BUFFER* ，要求返回的是BUFFER **,
	也就是BUFFER* 类型的地址了。
	看起来，这个list->list是个指针数组啊。
	*/
    return &list->list[i];
}

void SCOmx_bufferlist_remove(BUFFERLIST* list, OMX_U32 i)
{
    assert(list);
    assert(i < list->size);
	/*数组虽然可以随机删除
	但是空的位置要通过memove来补*/
	/*
	       #include <string.h>

       void *memmove(void *dest, const void *src, size_t n);

  The  memmove()  function  copies n bytes from memory area src to memory
       area dest.  The memory areas may overlap: copying takes place as though
       the  bytes in src are first copied into a temporary array that does not
       overlap src or dest, and the bytes are then copied from  the  temporary
       array to dest.

	*/
	//如果i是从0开始算的，那么i右边剩余的元素数目就是size -i -1
    memmove(list->list+i, list->list+i+1, (list->size-i-1)*sizeof(BUFFER*));
    --list->size;
}

//这个操作为啥只有size 置 0啊
void SCOmx_bufferlist_clear(BUFFERLIST* list)
{
    assert(list);
    list->size = 0;
}

OMX_BOOL SCOmx_bufferlist_push_back(BUFFERLIST* list, BUFFER* buff)
{
    assert(list);
    if (list->size == list->capacity)//初始化就是这样的啊，有啥错误�
        return OMX_FALSE;
    list->list[list->size++] = buff;
    return OMX_TRUE;
}
