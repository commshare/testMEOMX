#ifndef _SC_BUFFER_LIST_H_
#define _SC_BUFFER_LIST_H_
typedef struct BUFFER
{
    OMX_BUFFERHEADERTYPE* header;/**omx������������ݽṹ����ʾ�����?*/
    OMX_BUFFERHEADERTYPE  headerdata;
    OMX_U32               flags;
    OMX_U32               allocsize;
    OSAL_BUS_WIDTH        bus_address;
    OMX_U8*               bus_data;
} BUFFER;
typedef struct BUFFERLIST
{
    BUFFER** list;//�Ǹ����鰡����ͷ��BUFFER* ����ָ��
    OMX_U32  size; // list size
    OMX_U32  capacity;
}BUFFERLIST;

/*����*/
OMX_ERRORTYPE SCOmx_bufferlist_init(BUFFERLIST* list, OMX_U32 size);
/*Ԥ���*/
OMX_ERRORTYPE SCOmx_bufferlist_reserve(BUFFERLIST* list, OMX_U32 newsize);
/*�ͷ�*/
void          SCOmx_bufferlist_destroy(BUFFERLIST* list);
/*���*/
OMX_U32       SCOmx_bufferlist_get_size(BUFFERLIST* list);
/*����*/
OMX_U32       SCOmx_bufferlist_get_capacity(BUFFERLIST* list);
/*�����λ*/
BUFFER**      SCOmx_bufferlist_at(BUFFERLIST* list, OMX_U32 i);
/*ɾ��*/
void          SCOmx_bufferlist_remove(BUFFERLIST* list, OMX_U32 i);
/*���*/
void          SCOmx_bufferlist_clear(BUFFERLIST* list);
/*�ѵ����Ǵ�β������� ͷ��ȡ�� �*/
OMX_BOOL      SCOmx_bufferlist_push_back(BUFFERLIST* list, BUFFER* buff);

#endif
