
//--------------------------------------------------------------------------------
//--
//--  Description :  Port component
//--
//----------------------------------------------------------------------------*/


#include "sc_port.h"
#include "OSAL.h"

// nBufferCountMin is a read-only field that specifies the minimum number of
// buffers that the port requires. The component shall define this non-zero default
// value.

// nBufferCountActual represents the number of buffers that are required on
// this port before it is populated, as indicated by the bPopulated field of this
// structure. The component shall set a default value no less than
// nBufferCountMin for this field.

// nBufferSize is a read-only field that specifies the minimum size in bytes for
// buffers that are allocated for this port. .

 //OMX_U32 nBufferCountActual;    /**< The actual number of buffers allocated on this port */
 //   OMX_U32 nBufferCountMin;       /**< The minimum number of buffers this port requires */
 //   OMX_U32 nBufferSize;           /**< Size, in bytes, for buffers to be used for this channel */
OMX_ERRORTYPE SCOmx_port_init(PORT* p, OMX_U32 nBufferCountMin, OMX_U32 nBufferCountActual, OMX_U32 nBuffers, OMX_U32 nBufferSize)
{
  assert(p);
  	//可见，p的内存是在外头分配的
    memset(p, 0, sizeof(PORT));

    OMX_ERRORTYPE err = OMX_ErrorNone;
    err = OSAL_MutexCreate(&p->buffermutex);
    if (err != OMX_ErrorNone)
        goto INIT_FAIL;

	//event来了
    err = OSAL_EventCreate(&p->bufferevent);
    if (err != OMX_ErrorNone)
        goto INIT_FAIL;

    if (nBuffers)
    {
        err = SCOmx_bufferlist_init(&p->buffers, nBuffers);
        if (err != OMX_ErrorNone)
            goto INIT_FAIL;

        err = SCOmx_bufferlist_init(&p->bufferqueue, nBuffers);
        if (err != OMX_ErrorNone)
            goto INIT_FAIL;
    }
    p->def.nBufferCountActual = nBufferCountActual;//为这个port实际分配的缓冲数目
    p->def.nBufferCountMin    = nBufferCountMin;//这个port所请求的最小的缓冲数目
    p->def.nBufferSize        = nBufferSize;//这个channel所用的字节数目
    return OMX_ErrorNone;

 INIT_FAIL:
    if (p->buffermutex) //mutex的资源销毁
        OSAL_MutexDestroy(p->buffermutex);
    if (p->bufferevent)//event的资源销毁
        OSAL_EventDestroy(p->bufferevent);
	//即使init都还没 成功，也可以销毁
    SCOmx_bufferlist_destroy(&p->buffers);
    SCOmx_bufferlist_destroy(&p->bufferqueue);
    return err;

}

void     SCOmx_port_destroy(PORT* p){
 assert(p);

    OMX_U32 i = 0;
	//获取BUFFERLIST的最大值
    OMX_U32 x = SCOmx_bufferlist_get_size(&p->buffers);
    for (i=0; i<x; ++i)
    {
		/*右边返回BUFFERLIST中每个元素的地址，然后通过*,得到每个元素(元素是BUFFER*类型的)*/
        BUFFER* buff = *SCOmx_bufferlist_at(&p->buffers, i);
		//释放一块内存?
        OSAL_Free(buff);
    }
    SCOmx_bufferlist_destroy(&p->buffers);
    SCOmx_bufferlist_destroy(&p->bufferqueue);

    OMX_ERRORTYPE err;
    err = OSAL_MutexDestroy(p->buffermutex);
    assert(err == OMX_ErrorNone);
    err = OSAL_EventDestroy(p->bufferevent);
    assert(err == OMX_ErrorNone);

    memset(p, 0, sizeof(PORT));
}

OMX_BOOL SCOmx_port_is_allocated(PORT* p){
	return SCOmx_bufferlist_get_capacity(&p->buffers) > 0;
}

OMX_BOOL SCOmx_port_is_ready(PORT* p){
	 assert(p);
    if (!p->def.bEnabled)
        return OMX_TRUE;
    return p->def.bPopulated;
}
OMX_BOOL SCOmx_port_is_enabled(PORT* p){
	/*
	 Ports default to enabled and are enabled/disabled by
     OMX_CommandPortEnable/OMX_CommandPortDisable.
     When disabled a port is unpopulated. A disabled port
     is not populated with buffers on a transition to IDLE.
	*/
	 return p->def.bEnabled;
}
// Return true if port has allocated buffers, otherwise false.
//注意，这个has的意思是，预先分配的size是否存在，size是最大值，并不是当前值。
OMX_BOOL SCOmx_port_has_buffers(PORT* p){
 return SCOmx_bufferlist_get_size(&p->buffers) > 0;
}
OMX_BOOL SCOmx_port_is_supplier(PORT* p){
	 if (p->tunnelcomp == NULL)
        return OMX_FALSE;

	/*Direction (input or output) of this port*/
    if (p->def.eDir == OMX_DirInput &&
        p->tunnel.eSupplier == OMX_BufferSupplyInput)
        return OMX_TRUE;

    if (p->def.eDir == OMX_DirOutput &&
        p->tunnel.eSupplier == OMX_BufferSupplyOutput)
        return OMX_TRUE;

    return OMX_FALSE;
}
OMX_BOOL SCOmx_port_is_tunneled(PORT* p){
	 return p->tunnelcomp != NULL;
}
/*
是tunnel的，或者俩queue
*/
OMX_BOOL SCOmx_port_has_all_supplied_buffers(PORT* p){
	 if (p->tunnelcomp == NULL)
        return OMX_TRUE;

    if (SCOmx_port_buffer_count(p) == SCOmx_port_buffer_queue_count(p))
        return OMX_TRUE;

    return OMX_FALSE;
}
void     SCOmx_port_setup_tunnel(PORT* p, OMX_HANDLETYPE comp, OMX_U32 port, OMX_BUFFERSUPPLIERTYPE type){
    assert(p);
    p->tunnelcomp = comp;
    p->tunnelport = port;
    p->tunnel.nTunnelFlags = 0;
    p->tunnel.eSupplier    = type;
}

BUFFER*  SCOmx_port_find_buffer(PORT* p, OMX_BUFFERHEADERTYPE* header){
	OMX_U32 size = SCOmx_bufferlist_get_size(&p->buffers);
    if (size == 0)
        return NULL;
    OMX_U32 i=0;
    for (i=0; i<size; ++i)
    {
        BUFFER* buff = *SCOmx_bufferlist_at(&p->buffers, i);
        if (buff->header == header)
            return buff;
    }
    return NULL;
}


// Try to allocate next available buffer from the array of buffers associated with
// with the port. The finding is done by looking at the associated buffer flags and
// checking the BUFFER_FLAG_IN_USE flag.
//
// Returns OMX_TRUE if next buffer could be found. Otherwise OMX_FALSE, which
// means that all buffer headers are in use.
//新分配的BUFFER类型的内存，通过buff返回出来，此时buff是一个BUFFER * 类型的指针的地址。
OMX_BOOL SCOmx_port_allocate_next_buffer(PORT* p, BUFFER** buff){
	//这个新分配的内存叫做next的是指向BUFFER类型的指针
	BUFFER* next = (BUFFER*)OSAL_Malloc(sizeof(BUFFER));
    if (next==NULL)
        return OMX_FALSE;

    memset(next, 0, sizeof(BUFFER));
    next->flags |= BUFFER_FLAG_IN_USE;
    // hack for tunneling.
    // The buffer header is always accessed through a pointer. In normal case
    // it just points to the header object within the buffer. But in case
    // of tunneling it can be made to point to a header allocated by the tunneling component.
    /*  hack 隧道
    buffer的header通常是通过指针访问的。
    平时指针是指向buffer内的header对象的，但是如果是隧道模式下，指针就是指向
    隧道组件分配的header了。
	*/
    next->header = &next->headerdata;
    OMX_BOOL ret = SCOmx_bufferlist_push_back(&p->buffers, next);
    if (ret == OMX_FALSE)
    {
        OMX_ERRORTYPE err;
		//capacity是最大值
        OMX_U32 capacity = SCOmx_bufferlist_get_capacity(&p->buffers);
		//最大值至少为5
        if (capacity == 0) capacity = 5;
		//预定capacity * 2大小的该内存空间
        err = SCOmx_bufferlist_reserve(&p->buffers, capacity * 2);
        if (err != OMX_ErrorNone)
        {
            OSAL_Free(next);
            return OMX_FALSE;
        }
		//BUFFERLIST数组尾部插入一个元素
        SCOmx_bufferlist_push_back(&p->buffers, next);
    }
	//这个时候buff是next的地址啊
    *buff = next;
    return OMX_TRUE;
}


//释放BUFFERLIST中某个BUFFER * ，通过遍历和比较实现
OMX_BOOL SCOmx_port_release_buffer(PORT* p, BUFFER* buff){
	OMX_U32 i=0;
    OMX_U32 x=SCOmx_bufferlist_get_size(&p->buffers);
    for (i=0; i<x; ++i)
    {
        BUFFER** buffer = SCOmx_bufferlist_at(&p->buffers, i);
		//这是俩指针在比较啊
        if (*buffer == buff)
        {
			//释放指针
            OSAL_Free(buff);
			//移动数组的元素，覆盖内存
            SCOmx_bufferlist_remove(&p->buffers, i);
            return OMX_TRUE;
        }
    }
    return OMX_FALSE;
}
//释放PORT的BUFFERLIST数组的每个元素的内存
OMX_BOOL SCOmx_port_release_all_allocated(PORT* p){
	// release all allocated buffer objects
	//当前已经存储的元素数目
    OMX_U32 x=SCOmx_bufferlist_get_size(&p->buffers);
    OMX_U32 i=0;
    for (i=0; i<x; ++i)
    {
		//通过位置i得到BUFFER* 元素的内存地址，然后释放内存地址
        BUFFER** buffer = SCOmx_bufferlist_at(&p->buffers, i);
        OSAL_Free(*buffer);
    }
    SCOmx_bufferlist_clear(&p->buffers);
    return OMX_TRUE;
}
// Return how many buffers are allocated for this port. 已经存储了多少元素
OMX_U32 SCOmx_port_buffer_count(PORT* p)
{
    return SCOmx_bufferlist_get_size(&p->buffers);
}




/// queue functions


// Push next buffer into the port's buffer queue.
OMX_ERRORTYPE SCOmx_port_push_buffer(PORT* p, BUFFER* buff){
    OMX_ERRORTYPE err;
    OMX_U32 ret = SCOmx_bufferlist_push_back(&p->bufferqueue, buff);
    if (ret == OMX_FALSE)
    {
        OMX_U32 capacity = SCOmx_bufferlist_get_capacity(&p->bufferqueue);
        err = SCOmx_bufferlist_reserve(&p->bufferqueue, capacity * 2);
        if (err != OMX_ErrorNone)
            return err;
        SCOmx_bufferlist_push_back(&p->bufferqueue, buff);
    }
	//设定一个event
    err = OSAL_EventSet(p->bufferevent);
    if (err != OMX_ErrorNone)
    {
        OMX_U32 size = SCOmx_bufferlist_get_size(&p->bufferqueue);
        SCOmx_bufferlist_remove(&p->bufferqueue, size-1);
        return err;
    }
    return OMX_ErrorNone;
}

// Get next buffer from the port's buffer queue.
OMX_BOOL SCOmx_port_get_buffer(PORT* p, BUFFER** buff){
	OMX_U32 size = SCOmx_bufferlist_get_size(&p->bufferqueue);
    if (size == 0)
    {
        *buff = NULL;
        return OMX_FALSE;
    }
	//只要是size不为0，就可以从bufferqueue中直接取
    *buff = *SCOmx_bufferlist_at(&p->bufferqueue, 0);
    return OMX_TRUE;
}

OMX_BOOL SCOmx_port_get_allocated_buffer_at(PORT* p, BUFFER** buff, OMX_U32 i)
{
    OMX_U32 size = SCOmx_bufferlist_get_size(&p->buffers);
    if (!(i < size))
    {
        *buff = NULL;
        return OMX_FALSE;
    }
    *buff = *SCOmx_bufferlist_at(&p->buffers, i);
    return OMX_TRUE;
}
OMX_BOOL SCOmx_port_pop_buffer(PORT* p)
{
    OMX_U32 size = SCOmx_bufferlist_get_size(&p->bufferqueue);
    if (size==0)
        return OMX_FALSE;

    if (size-1 == 0)
    {
		//为毛只有一个元素的时候，要先充值下event啊
        OMX_ERRORTYPE err = OSAL_EventReset(p->bufferevent);
        if (err != OMX_ErrorNone)
            return OMX_FALSE;
    }
    SCOmx_bufferlist_remove(&p->bufferqueue, 0);
    return OMX_TRUE;
}
// Lock the buffer queue
OMX_ERRORTYPE SCOmx_port_lock_buffers(PORT* p){
 	  assert(p);
    return OSAL_MutexLock(p->buffermutex);
}
// Unlock the buffer queue
OMX_ERRORTYPE SCOmx_port_unlock_buffers(PORT* p){
	 assert(p);
    return OSAL_MutexUnlock(p->buffermutex);
}


OMX_U32 SCOmx_port_buffer_queue_count(PORT* p)
{
    return SCOmx_bufferlist_get_size(&p->bufferqueue);
}
void SCOmx_port_buffer_queue_clear(PORT* p){
	 OMX_U32 size = SCOmx_bufferlist_get_size(&p->bufferqueue);
    if (size == 0)
        return;
    OSAL_EventReset(p->bufferevent);
    SCOmx_bufferlist_clear(&p->bufferqueue);
}
