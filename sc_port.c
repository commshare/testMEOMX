
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
  	//�ɼ���p���ڴ�������ͷ�����
    memset(p, 0, sizeof(PORT));

    OMX_ERRORTYPE err = OMX_ErrorNone;
    err = OSAL_MutexCreate(&p->buffermutex);
    if (err != OMX_ErrorNone)
        goto INIT_FAIL;

	//event����
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
    p->def.nBufferCountActual = nBufferCountActual;//Ϊ���portʵ�ʷ���Ļ�����Ŀ
    p->def.nBufferCountMin    = nBufferCountMin;//���port���������С�Ļ�����Ŀ
    p->def.nBufferSize        = nBufferSize;//���channel���õ��ֽ���Ŀ
    return OMX_ErrorNone;

 INIT_FAIL:
    if (p->buffermutex) //mutex����Դ����
        OSAL_MutexDestroy(p->buffermutex);
    if (p->bufferevent)//event����Դ����
        OSAL_EventDestroy(p->bufferevent);
	//��ʹinit����û �ɹ���Ҳ��������
    SCOmx_bufferlist_destroy(&p->buffers);
    SCOmx_bufferlist_destroy(&p->bufferqueue);
    return err;

}

void     SCOmx_port_destroy(PORT* p){
 assert(p);

    OMX_U32 i = 0;
	//��ȡBUFFERLIST�����ֵ
    OMX_U32 x = SCOmx_bufferlist_get_size(&p->buffers);
    for (i=0; i<x; ++i)
    {
		/*�ұ߷���BUFFERLIST��ÿ��Ԫ�صĵ�ַ��Ȼ��ͨ��*,�õ�ÿ��Ԫ��(Ԫ����BUFFER*���͵�)*/
        BUFFER* buff = *SCOmx_bufferlist_at(&p->buffers, i);
		//�ͷ�һ���ڴ�?
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
//ע�⣬���has����˼�ǣ�Ԥ�ȷ����size�Ƿ���ڣ�size�����ֵ�������ǵ�ǰֵ��
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
��tunnel�ģ�������queue
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
//�·����BUFFER���͵��ڴ棬ͨ��buff���س�������ʱbuff��һ��BUFFER * ���͵�ָ��ĵ�ַ��
OMX_BOOL SCOmx_port_allocate_next_buffer(PORT* p, BUFFER** buff){
	//����·�����ڴ����next����ָ��BUFFER���͵�ָ��
	BUFFER* next = (BUFFER*)OSAL_Malloc(sizeof(BUFFER));
    if (next==NULL)
        return OMX_FALSE;

    memset(next, 0, sizeof(BUFFER));
    next->flags |= BUFFER_FLAG_IN_USE;
    // hack for tunneling.
    // The buffer header is always accessed through a pointer. In normal case
    // it just points to the header object within the buffer. But in case
    // of tunneling it can be made to point to a header allocated by the tunneling component.
    /*  hack ���
    buffer��headerͨ����ͨ��ָ����ʵġ�
    ƽʱָ����ָ��buffer�ڵ�header����ģ�������������ģʽ�£�ָ�����ָ��
    �����������header�ˡ�
	*/
    next->header = &next->headerdata;
    OMX_BOOL ret = SCOmx_bufferlist_push_back(&p->buffers, next);
    if (ret == OMX_FALSE)
    {
        OMX_ERRORTYPE err;
		//capacity�����ֵ
        OMX_U32 capacity = SCOmx_bufferlist_get_capacity(&p->buffers);
		//���ֵ����Ϊ5
        if (capacity == 0) capacity = 5;
		//Ԥ��capacity * 2��С�ĸ��ڴ�ռ�
        err = SCOmx_bufferlist_reserve(&p->buffers, capacity * 2);
        if (err != OMX_ErrorNone)
        {
            OSAL_Free(next);
            return OMX_FALSE;
        }
		//BUFFERLIST����β������һ��Ԫ��
        SCOmx_bufferlist_push_back(&p->buffers, next);
    }
	//���ʱ��buff��next�ĵ�ַ��
    *buff = next;
    return OMX_TRUE;
}


//�ͷ�BUFFERLIST��ĳ��BUFFER * ��ͨ�������ͱȽ�ʵ��
OMX_BOOL SCOmx_port_release_buffer(PORT* p, BUFFER* buff){
	OMX_U32 i=0;
    OMX_U32 x=SCOmx_bufferlist_get_size(&p->buffers);
    for (i=0; i<x; ++i)
    {
        BUFFER** buffer = SCOmx_bufferlist_at(&p->buffers, i);
		//������ָ���ڱȽϰ�
        if (*buffer == buff)
        {
			//�ͷ�ָ��
            OSAL_Free(buff);
			//�ƶ������Ԫ�أ������ڴ�
            SCOmx_bufferlist_remove(&p->buffers, i);
            return OMX_TRUE;
        }
    }
    return OMX_FALSE;
}
//�ͷ�PORT��BUFFERLIST�����ÿ��Ԫ�ص��ڴ�
OMX_BOOL SCOmx_port_release_all_allocated(PORT* p){
	// release all allocated buffer objects
	//��ǰ�Ѿ��洢��Ԫ����Ŀ
    OMX_U32 x=SCOmx_bufferlist_get_size(&p->buffers);
    OMX_U32 i=0;
    for (i=0; i<x; ++i)
    {
		//ͨ��λ��i�õ�BUFFER* Ԫ�ص��ڴ��ַ��Ȼ���ͷ��ڴ��ַ
        BUFFER** buffer = SCOmx_bufferlist_at(&p->buffers, i);
        OSAL_Free(*buffer);
    }
    SCOmx_bufferlist_clear(&p->buffers);
    return OMX_TRUE;
}
// Return how many buffers are allocated for this port. �Ѿ��洢�˶���Ԫ��
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
	//�趨һ��event
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
	//ֻҪ��size��Ϊ0���Ϳ��Դ�bufferqueue��ֱ��ȡ
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
		//Ϊëֻ��һ��Ԫ�ص�ʱ��Ҫ�ȳ�ֵ��event��
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
