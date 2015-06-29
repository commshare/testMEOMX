#ifndef __SC_MSG_QUEUE_H__
#define __SC_MSG_QUEUE_H__

#include <OMX_Types.h>
#include <OMX_Core.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msg_node_s msg_node_t;
struct msg_node_s //��Ϣ�����еĽڵ�
{
    msg_node_t* next; //��һ���ڵ�
    msg_node_t* prev; //ǰһ���ڵ�
    OMX_PTR   data; //����ָ��
};
typedef struct msgque_s{
    msg_node_t*      head;//����ͷ
    msg_node_t*      tail;//����β
    OMX_U32        size; //���д�С
    OMX_HANDLETYPE mutex; //����
    OMX_HANDLETYPE event; //�¼�
} msgque_t;


// Initialize a new message queue instance ��ʼ��һ����Ϣ����ʵ��
OMX_ERRORTYPE sc_msgque_init(OMX_IN msgque_t* q);

// Destroy the message queue instance, free allocated resources
void sc_msgque_destroy(OMX_IN msgque_t* q);


// Push a new message at the end of the queue. ��һ����Ϣ���ڶ���β��
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
