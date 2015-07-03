

/*------------------------------------------------------------------------------
    OSAL_AllocatorInit
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_AllocatorInit(OSAL_ALLOCATOR* alloc)
{
#ifdef OMX_MEM_TRC

        pf = fopen("omx_mem_trc.csv", "w");
        if(pf)
            fprintf(pf,
            "linead memory usage in bytes;linear memory usage in 4096 pages;\n");

#endif
#ifdef USE_DWL
    if(pdwl)
        return OSAL_ERRORNONE;

    LOGD("OSAL_Init\n");

    DWLInitParam_t dwlInit;

    dwlInit.clientType = DWL_CLIENT_TYPE_H264_DEC;

    pdwl = DWLInit(&dwlInit);


    // NOTE: error handling
    return OSAL_ERRORNONE;
#endif
#ifdef MEMALLOCHW
    OSAL_ERRORTYPE err = OSAL_ERRORNONE;
    // open memalloc for linear memory allocation
    alloc->fd_memalloc = open("/dev/memalloc", O_RDWR | O_SYNC);
    if (alloc->fd_memalloc == -1)
    {
        TRACE_PRINT("memalloc not found\n");
        err = OSAL_ERROR_UNDEFINED;
        goto FAIL;
    }
    // open raw memory for memory mapping
    alloc->fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if (alloc->fd_mem == -1)
    {
        TRACE_PRINT("uio0 not found\n");
        err = OSAL_ERROR_UNDEFINED;
        goto FAIL;
    }
    return OSAL_ERRORNONE;
 FAIL:
    if (alloc->fd_memalloc > 0) close(alloc->fd_memalloc);
    if (alloc->fd_mem > 0)      close(alloc->fd_mem);
    return err;
#else
    return OSAL_ERRORNONE;
#endif
}

/*------------------------------------------------------------------------------
    OSAL_AllocatorDestroy
------------------------------------------------------------------------------*/
void OSAL_AllocatorDestroy(OSAL_ALLOCATOR* alloc)
{
#ifdef OMX_MEM_TRC

    if(pf)
    {
        fclose(pf);
        pf =NULL;
    }

#endif
#ifdef USE_DWL
    assert(alloc);
    DWLRelease(pdwl);
    pdwl = NULL;
#endif
#ifdef MEMALLOCHW
    assert(alloc->fd_memalloc > 0);
    assert(alloc->fd_mem > 0);
    int ret = 0;
    ret = close(alloc->fd_memalloc);
    assert( ret == 0 );
    ret= close(alloc->fd_mem);
    assert( ret == 0 );

    alloc->fd_memalloc = 0;
    alloc->fd_mem      = 0;
#endif
}


/*------------------------------------------------------------------------------
    OSAL_AllocatorAllocMem
------------------------------------------------------------------------------*/
OSAL_ERRORTYPE OSAL_AllocatorAllocMem(OSAL_ALLOCATOR* alloc, OSAL_U32* size,
        OSAL_U8** bus_data, OSAL_BUS_WIDTH* bus_address)
{
#ifdef OMX_MEM_TRC

    if(pf)
        fprintf(pf, "%d bytes; %d chunks;\n", *size, *size/4096);

#endif
#ifdef USE_DWL

    DWLLinearMem_t info;
 //   memset(&info, 0, sizeof(DWLLinearMem_t));
    LOGD("OSAL_AllocatorAllocMem size=%d\n",*size);

    if(pdwl == 0)
    {
        OSAL_ALLOCATOR a;
        OSAL_AllocatorInit(&a);
    }

    int ret = DWLMallocLinear(pdwl, *size, &info);
    if (ret == 0)
    {
        *bus_data = (OSAL_U8*)info.virtualAddress;
        *bus_address = (OSAL_BUS_WIDTH)info.busAddress;
        LOGD("OSAL_AllocatorAllocMem OK\n    bus addr = 0x%08x\n    vir addr = 0x%08x\n",info.busAddress, info.virtualAddress);
        memset(info.virtualAddress, 0xab, *size);
        return OSAL_ERRORNONE;
    }
    else
    {
        LOGE("MallocLinear error %d\n",ret);
        return OSAL_ERROR_UNDEFINED;
    }
#endif
#ifdef MEMALLOCHW
    assert(alloc->fd_memalloc > 0);
    assert(alloc->fd_mem > 0);
    int pgsize = getpagesize();

    MemallocParams params;
    memset(&params, 0, sizeof(MemallocParams));

    *size = (*size + pgsize) & (~(pgsize - 1));
    params.size = *size;
    *bus_data   = MAP_FAILED;
    // get linear memory buffer
    ioctl(alloc->fd_memalloc, MEMALLOC_IOCXGETBUFFER, &params);
    if (params.busAddress == 0)
    {
        TRACE_PRINT("OSAL_ERROR_INSUFFICIENT_RESOURCES\n");
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;
    }
    // map the bus address to a virtual address

    TRACE_PRINT("alloc success. bus addr = 0x%x\n",params.busAddress);

    TRACE_PRINT("mmap(0, %d, %x, %x, %d, 0x%x);\n", *size, PROT_READ | PROT_WRITE, MAP_SHARED, alloc->fd_mem, params.busAddress);
    *bus_data = (OSAL_U8*)mmap(0, *size, PROT_READ | PROT_WRITE, MAP_SHARED, alloc->fd_mem, params.busAddress);
    if (*bus_data == MAP_FAILED)
    {
        TRACE_PRINT("mmap failed %s\n",strerror(errno));
        return OSAL_ERROR_UNDEFINED;
    }

    TRACE_PRINT("mmap success. vir addr = 0x%x\n",*bus_data);
    memset(*bus_data, 0, *size);

    TRACE_PRINT("memset OK\n");
    *bus_address = params.busAddress;
    return OSAL_ERRORNONE;
#else
    OSAL_U32 extra = sizeof(OSAL_U32);
    OSAL_U8* data  = (OSAL_U8*)malloc(*size + extra);
    if (data==NULL)
        return OSAL_ERROR_INSUFFICIENT_RESOURCES;

    OSAL_U32 sentinel = MEMORY_SENTINEL;
    // copy sentinel at the end of mem block
    memcpy(&data[*size], &sentinel, sizeof(OSAL_U32));

    *bus_data    = data;
    *bus_address = (OSAL_BUS_WIDTH)data;
    return OSAL_ERRORNONE;
#endif
}

/*------------------------------------------------------------------------------
    OSAL_AllocatorFreeMem
------------------------------------------------------------------------------*/
void OSAL_AllocatorFreeMem(OSAL_ALLOCATOR* alloc, OSAL_U32 size,
        OSAL_U8* bus_data, OSAL_BUS_WIDTH bus_address)
{
#ifdef USE_DWL
    LOGD("OSAL_AllocatorFreeMem\b");
    DWLLinearMem_t * info = malloc(sizeof(DWLLinearMem_t));
    info->size = size;
    info->virtualAddress = bus_data;
    info->busAddress = bus_address;

 //    DWLFreeLinear(pdwl, info); TODO
     LOGD("OSAL_AllocatorFreeMem %x ok\n", bus_address);
     return OSAL_ERRORNONE;
#endif
#ifdef MEMALLOCHW
    assert(alloc->fd_memalloc > 0);
    assert(alloc->fd_mem > 0);

    if (bus_address)
        ioctl(alloc->fd_memalloc, MEMALLOC_IOCSFREEBUFFER, &bus_address);
    if (bus_data != MAP_FAILED)
        munmap(bus_data, size);

#else
    assert(((OSAL_BUS_WIDTH)bus_data) == bus_address);
    OSAL_U32 sentinel = MEMORY_SENTINEL;
    assert(memcmp(&bus_data[size], &sentinel, sizeof(OSAL_U32)) == 0 &&
            "memory corruption detected");
    free(bus_data);
#endif
}

/*------------------------------------------------------------------------------
    OSAL_AllocatorIsReady
------------------------------------------------------------------------------*/
OSAL_BOOL OSAL_AllocatorIsReady(const OSAL_ALLOCATOR* alloc)
{
#ifdef MEMALLOCHW
    if (alloc->fd_memalloc > 0 &&
        alloc->fd_mem > 0)
        return 1;
    return 0;
#endif

#ifdef USE_DWL
    if(pdwl)
        return 1;
    else
        return 0;
#endif

    return 1;
}

