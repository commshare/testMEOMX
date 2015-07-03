#ifndef _SC_BUFFER_LIST_H_
#define _SC_BUFFER_LIST_H_
typedef struct BUFFER
{
    OMX_BUFFERHEADERTYPE* header;/**omx≤ªª·”√’‚∏ˆ ˝æ›Ω·ππ¿¥±Ì æª∫≥Â∞…?*/
    OMX_BUFFERHEADERTYPE  headerdata;
    OMX_U32               flags;
    OMX_U32               allocsize;
    OSAL_BUS_WIDTH        bus_address;
    OMX_U8*               bus_data;
} BUFFER;
typedef struct BUFFERLIST
{
    BUFFER** list;// «∏ˆ ˝◊È∞°£¨¿ÔÕ∑ «BUFFER* ¿‡–Õ÷∏’Î
    OMX_U32  size; // list size
    OMX_U32  capacity;
}BUFFERLIST;

/*∑÷≈‰*/
OMX_ERRORTYPE SCOmx_bufferlist_init(BUFFERLIST* list, OMX_U32 size);
/*‘§¡ÙÂ*/
OMX_ERRORTYPE SCOmx_bufferlist_reserve(BUFFERLIST* list, OMX_U32 newsize);
/* Õ∑≈*/
void          SCOmx_bufferlist_destroy(BUFFERLIST* list);
/*◊Ó¥Û*/
OMX_U32       SCOmx_bufferlist_get_size(BUFFERLIST* list);
/*ø…”√*/
OMX_U32       SCOmx_bufferlist_get_capacity(BUFFERLIST* list);
/*ÀÊª˙∂®Œª*/
BUFFER**      SCOmx_bufferlist_at(BUFFERLIST* list, OMX_U32 i);
/*…æ≥˝*/
void          SCOmx_bufferlist_remove(BUFFERLIST* list, OMX_U32 i);
/*«Âø’*/
void          SCOmx_bufferlist_clear(BUFFERLIST* list);
/*ƒ—µ¿”÷ «¥”Œ≤≤ø≤Â»ÎÂ Õ∑≤ø»°≥ˆ Â*/
OMX_BOOL      SCOmx_bufferlist_push_back(BUFFERLIST* list, BUFFER* buff);

#endif
