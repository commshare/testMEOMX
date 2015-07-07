#ifndef SC_PORT_H
#define SC_PORT_H

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Video.h>
#include "OSAL.h"
#include"sc_bufferlist.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_FLAG_IN_USE      0x1
#define BUFFER_FLAG_MY_BUFFER   0x2
#define BUFFER_FLAG_IS_TUNNELED 0x4
#define BUFFER_FLAG_MARK        0x8



/*封装OMX的port*/
typedef struct PORT
{
    OMX_PARAM_PORTDEFINITIONTYPE def;          // OMX port definition
    OMX_TUNNELSETUPTYPE          tunnel; /*隧道化（Tunneled）：让两个组件直接连接的方式*/
	/*用于处理tunneled组件的句柄*/
    OMX_HANDLETYPE               tunnelcomp;   // handle to the tunneled component
    OMX_U32                      tunnelport;   // port index of the tunneled components port
	//封装了BUFFER**
	BUFFERLIST                   buffers;      // buffers for this port
    BUFFERLIST                   bufferqueue;  // buffers queued up for processing
    OMX_HANDLETYPE               buffermutex;  // mutex to protect the buffer queue
    OMX_HANDLETYPE               bufferevent;  // event object for buffer queue
} PORT;


// nBufferCountMin is a read-only field that specifies the minimum number of
// buffers that the port requires. The component shall define this non-zero default
// value.

// nBufferCountActual represents the number of buffers that are required on
// this port before it is populated, as indicated by the bPopulated field of this
// structure. The component shall set a default value no less than
// nBufferCountMin for this field.

// nBufferSize is a read-only field that specifies the minimum size in bytes for
// buffers that are allocated for this port. .
OMX_ERRORTYPE SCOmx_port_init(PORT* p, OMX_U32 nBufferCountMin, OMX_U32 nBufferCountActual, OMX_U32 nBuffers, OMX_U32 buffersize);

void     SCOmx_port_destroy(PORT* p);

OMX_BOOL SCOmx_port_is_allocated(PORT* p);

OMX_BOOL SCOmx_port_is_ready(PORT* p);

OMX_BOOL SCOmx_port_is_enabled(PORT* p);

// Return true if port has allocated buffers, otherwise false.
OMX_BOOL SCOmx_port_has_buffers(PORT* p);

OMX_BOOL SCOmx_port_is_supplier(PORT* p);
OMX_BOOL SCOmx_port_is_tunneled(PORT* p);

OMX_BOOL SCOmx_port_has_all_supplied_buffers(PORT* p);

void     SCOmx_port_setup_tunnel(PORT* p, OMX_HANDLETYPE comp, OMX_U32 port, OMX_BUFFERSUPPLIERTYPE type);


BUFFER*  SCOmx_port_find_buffer(PORT* p, OMX_BUFFERHEADERTYPE* header);


// Try to allocate next available buffer from the array of buffers associated with
// with the port. The finding is done by looking at the associated buffer flags and
// checking the BUFFER_FLAG_IN_USE flag.
//
// Returns OMX_TRUE if next buffer could be found. Otherwise OMX_FALSE, which
// means that all buffer headers are in use.
OMX_BOOL SCOmx_port_allocate_next_buffer(PORT* p, BUFFER** buff);



//
OMX_BOOL SCOmx_port_release_buffer(PORT* p, BUFFER* buff);

//
OMX_BOOL SCOmx_port_release_all_allocated(PORT* p);


// Return how many buffers are allocated for this port.
OMX_U32 SCOmx_port_buffer_count(PORT* p);


// Get an allocated buffer.
OMX_BOOL SCOmx_port_get_allocated_buffer_at(PORT* p, BUFFER** buff, OMX_U32 i);


/// queue functions


// Push next buffer into the port's buffer queue.
OMX_ERRORTYPE SCOmx_port_push_buffer(PORT* p, BUFFER* buff);


// Get next buffer from the port's buffer queue.
OMX_BOOL SCOmx_port_get_buffer(PORT* p, BUFFER** buff);


// Get a buffer at a certain location from port's buffer queue
OMX_BOOL SCOmx_port_get_buffer_at(PORT* P, BUFFER** buff, OMX_U32 i);


// Pop off the first buffer from the port's buffer queue
OMX_BOOL      SCOmx_port_pop_buffer(PORT* p);

// Lock the buffer queue
OMX_ERRORTYPE SCOmx_port_lock_buffers(PORT* p);
// Unlock the buffer queue
OMX_ERRORTYPE SCOmx_port_unlock_buffers(PORT* p);


// Return how many buffers are queued for this port.
OMX_U32 SCOmx_port_buffer_queue_count(PORT* p);

void SCOmx_port_buffer_queue_clear(PORT* p);

#ifdef __cplusplus
}
#endif

#endif
