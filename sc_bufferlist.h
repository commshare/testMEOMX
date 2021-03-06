#ifndef _SC_BUFFER_LIST_H_
#define _SC_BUFFER_LIST_H_
typedef struct BUFFER
{
    OMX_BUFFERHEADERTYPE* header;/**omx不会用这个数据结构来表示缓冲吧?*/
    OMX_BUFFERHEADERTYPE  headerdata;
    OMX_U32               flags;
    OMX_U32               allocsize;
    OSAL_BUS_WIDTH        bus_address;
    OMX_U8*               bus_data;
} BUFFER;
typedef struct BUFFERLIST
{
    BUFFER** list;//是个数组啊，里头是BUFFER* 类型指针
    OMX_U32  size; // list size
    OMX_U32  capacity;
}BUFFERLIST;

/*分配*/
OMX_ERRORTYPE SCOmx_bufferlist_init(BUFFERLIST* list, OMX_U32 size);
/*预留�*/
OMX_ERRORTYPE SCOmx_bufferlist_reserve(BUFFERLIST* list, OMX_U32 newsize);
/*释放*/
void          SCOmx_bufferlist_destroy(BUFFERLIST* list);
/*最大*/
OMX_U32       SCOmx_bufferlist_get_size(BUFFERLIST* list);
/*可用*/
OMX_U32       SCOmx_bufferlist_get_capacity(BUFFERLIST* list);
/*随机定位*/
BUFFER**      SCOmx_bufferlist_at(BUFFERLIST* list, OMX_U32 i);
/*删除*/
void          SCOmx_bufferlist_remove(BUFFERLIST* list, OMX_U32 i);
/*清空*/
void          SCOmx_bufferlist_clear(BUFFERLIST* list);
/*难道又是从尾部插入� 头部取出 �*/
OMX_BOOL      SCOmx_bufferlist_push_back(BUFFERLIST* list, BUFFER* buff);

#endif
