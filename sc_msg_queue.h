#ifndef __SC_MSG_QUEUE_H__
#define __SC_MSG_QUEUE_H__

#include <OMX_Types.h>
#include <OMX_Core.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msg_node_s msg_node_t;
struct msg_node_s //消息队列中的节点
{
    msg_node_t* next; //后一个节点
    msg_node_t* prev; //前一个节点
    OMX_PTR   data; //数据指针
};
typedef struct msgque_s{
    msg_node_t*      head;//队列头
    msg_node_t*      tail;//队列尾
    OMX_U32        size; //队列大小
    OMX_HANDLETYPE mutex; //互斥
    OMX_HANDLETYPE event; //事件
} msgque_t;


// Initialize a new message queue instance 初始化一个消息队列实例
OMX_ERRORTYPE sc_msgque_init(OMX_IN msgque_t* q);

// Destroy the message queue instance, free allocated resources
void sc_msgque_destroy(OMX_IN msgque_t* q);


// Push a new message at the end of the queue. 把一个消息放在队列尾部
// Function provides commit/rollback semantics.
OMX_ERRORTYPE sc_msgque_push_back(OMX_IN msgque_t* q, OMX_IN OMX_PTR ptr);

// Get a message from the front, returns always immediately but
// ptr will point to NULL if the queue is empty.
// Function provides commit/rollback semantics.
OMX_ERRORTYPE sc_msgque_get_front(OMX_IN msgque_t* q, OMX_OUT OMX_PTR* ptr);

// Get current queue size
OMX_ERRORTYPE sc_msgque_get_size(OMX_IN msgque_t* q, OMX_OUT OMX_U32* size);



#endif
#endif
