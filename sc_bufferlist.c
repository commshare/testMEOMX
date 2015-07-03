
#include <assert.h>
#include <string.h>

typedef BUFFER** swap_type;
static  /*��������ָ�밡*/
	//����������ָ�룬��ʹ���У�������ǵ�ַ��
void swap_ptr(swap_type* one, swap_type* two)
{
    swap_type temp = *one;//ȡ��ַ���������ǰ�ָ��ָ�������ȡ����
    *one = *two;
    *two = temp; //ָ��twoָ������ݣ������һ���µ�ֵ��
}

OMX_ERRORTYPE SCOmx_bufferlist_init(BUFFERLIST* list, OMX_U32 size)
{ assert(list);

	/*
	BUFFER** ÿ��Ԫ����һ��BUFFER* ����ָ�롣ΪBUFFER**����size����СΪBUFFER*���ʹ�С
	�Ŀռ䡣
	*/
    list->list = (BUFFER**)OSAL_Malloc(sizeof(BUFFER*) * size);
    if (!list->list)
        return OMX_ErrorInsufficientResources;

    memset(list->list, 0, sizeof(BUFFER*) * size);
    list->size     = 0;
    list->capacity = size;//��ʼ״̬�£����õ������
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SCOmx_bufferlist_reserve(BUFFERLIST* list, OMX_U32 newsize)
{
    assert(list);
    if (newsize < list->capacity)//�´�С������ڵ��ڿ���
        return OMX_ErrorBadParameter;

    BUFFER** data = (BUFFER**)OSAL_Malloc(sizeof(BUFFER**) * newsize);
    if (!data)
        return OMX_ErrorInsufficientResources;

    memset(data, 0, sizeof(BUFFER*) * newsize);
    memcpy(data, list->list, list->size * sizeof(BUFFER*));

	//���������ָ��ĵ�ַ����������������ѵ�ַָ������ݽ����ˡ�
	//רҵlist->list����ָ����һ���µĵ�ַ��
    swap_ptr(&data, &list->list);

    list->capacity = newsize;
	//֮ǰ�Ƕοռ�Ϳ����ͷ��ˡ�
    OSAL_Free(data);
    return OMX_ErrorNone;
}

void SCOmx_bufferlist_destroy(BUFFERLIST* list)
{
    assert(list);
    if (list->list)
        OSAL_Free(list->list);
	//listָ��ָ����ڴ�ռ䱻����Ϊ0����freeȻ����ָ��ΪNULLô�
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
	ע�⣬Ϊɶ����Ҫ���ص��ǵ�ַ�
	��Ϊ��list->list��BUFFER* ����ָ�룬list->list[i]�õ��ľ���BUFFER* ��Ҫ�󷵻ص���BUFFER **,
	Ҳ����BUFFER* ���͵ĵ�ַ�ˡ�
	�����������list->list�Ǹ�ָ�����鰡��
	*/
    return &list->list[i];
}

void SCOmx_bufferlist_remove(BUFFERLIST* list, OMX_U32 i)
{
    assert(list);
    assert(i < list->size);
	/*������Ȼ�������ɾ��
	���ǿյ�λ��Ҫͨ��memove����*/
	/*
	       #include <string.h>

       void *memmove(void *dest, const void *src, size_t n);

  The  memmove()  function  copies n bytes from memory area src to memory
       area dest.  The memory areas may overlap: copying takes place as though
       the  bytes in src are first copied into a temporary array that does not
       overlap src or dest, and the bytes are then copied from  the  temporary
       array to dest.

	*/
	//���i�Ǵ�0��ʼ��ģ���ôi�ұ�ʣ���Ԫ����Ŀ����size -i -1
    memmove(list->list+i, list->list+i+1, (list->size-i-1)*sizeof(BUFFER*));
    --list->size;
}

//�������Ϊɶֻ��size �� 0��
void SCOmx_bufferlist_clear(BUFFERLIST* list)
{
    assert(list);
    list->size = 0;
}

OMX_BOOL SCOmx_bufferlist_push_back(BUFFERLIST* list, BUFFER* buff)
{
    assert(list);
    if (list->size == list->capacity)//��ʼ�����������İ�����ɶ�����
        return OMX_FALSE;
    list->list[list->size++] = buff;
    return OMX_TRUE;
}
