#include "dwl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* wrapper information */
typedef struct hRK70dwl {
    RK_U32 clientType;
    int fd;                  /* decoder device file */
    int fd_mem;              /* /dev/mem for mapping */
    int fd_memalloc;         /* linear memory allocator */
    volatile RK_U32 *pRegBase;  /* IO mem base */
    RK_U32 regSize;             /* IO mem size */
    RK_U32 freeLinMem;          /* Start address of free linear memory */
    RK_U32 freeRefFrmMem;       /* Start address of free reference frame memory */
    int semid;
    int sigio_needed;
#ifdef INTERNAL_TEST
    FILE *regDump;
#endif
} hRK70dwl_t;

/* HW ID retriving, static implementation */
RK_U32 DWLReadAsicID(void)
{
    return 0;
}

/* HW configuration retrieving, static implementation */
void DWLReadAsicConfig(DWLHwConfig_t * pHwCfg)
{

    pHwCfg->jpegSupport = JPEG_BASELINE;
    pHwCfg->jpegESupport = JPEG_EXT_SUPPORTED;

}

/* HW fuse retrieving, static implementation */
void DWLReadAsicFuseStatus(DWLHwFuseStatus_t * pHwFuseSts)
{
}

/* DWL initilaization and release */
const void *DWLInit(DWLInitParam_t * param)
{

    hRK70dwl_t *dec_dwl;

    dec_dwl = (hRK70dwl_t *) malloc(sizeof(hRK70dwl_t));

    return dec_dwl;
}

RK_S32 DWLRelease(const void *instance)
{
    hRK70dwl_t *dec_dwl = (hRK70dwl_t *) instance;
    free(dec_dwl);

    return (DWL_OK);
}

/* HW sharing */
RK_S32 DWLReserveHw(const void *instance)
{
    return DWL_OK;
}
void DWLReleaseHw(const void *instance)
{
    return;
}

/* Frame buffers memory */
RK_S32 DWLMallocRefFrm(const void *instance, RK_U32 size, DWLLinearMem_t * info)
{

    return DWLMallocLinear(instance, size, info);
}
void DWLFreeRefFrm(const void *instance, DWLLinearMem_t * info)
{

    DWLFreeLinear(instance, info);
}

/* SW/HW shared memory */
RK_S32 DWLMallocLinear(const void *instance, RK_U32 size, DWLLinearMem_t * info)
{

    info->size = size;
    info->virtualAddress = (RK_U32 *)malloc(size);
    info->busAddress = 0x00010000;

    return DWL_OK;
}

void DWLFreeLinear(const void *instance, DWLLinearMem_t * info)
{

    free(info->virtualAddress);
    return;
}

/* D-Cache coherence */
void DWLDCacheRangeFlush(const void *instance, DWLLinearMem_t * info)
{
    return;
}  /* NOT in use */

void DWLDCacheRangeRefresh(const void *instance, DWLLinearMem_t * info)
{
    return;
}    /* NOT in use */



/* Register access */
void DWLWriteReg(const void *instance, RK_U32 offset, RK_U32 value)
{
    return;
}


RK_U32 DWLReadReg(const void *instance, RK_U32 offset)
{
    return 0;
}

void DWLWriteRegAll(const void *instance, const RK_U32 * table, RK_U32 size)
{
    return;
} /* NOT in use */


void DWLReadRegAll(const void *instance, RK_U32 * table, RK_U32 size)
{
    return;
}    /* NOT in use */

/* HW starting/stopping */
void DWLEnableHW(const void *instance, RK_U32 offset, RK_U32 value)
{
    return;
}


void DWLDisableHW(const void *instance, RK_U32 offset, RK_U32 value)
{
    return;
}

/* HW synchronization */
RK_S32 DWLWaitHwReady(const void *instance, RK_U32 timeout)
{

    return DWL_OK;
}

/* SW/SW shared memory */
void *DWLmalloc(RK_U32 n)
{
    return malloc(n);
}


void DWLfree(void *p)
{
    free(p);
}


void *DWLcalloc(RK_U32 n, RK_U32 s)
{
    return calloc(n, s);
}

void *DWLmemcpy(void *d, const void *s, RK_U32 n)
{
    return memcpy(d, s, n);
}


void *DWLmemset(void *d, RK_S32 c, RK_U32 n)
{
    return memset(d, c, n);
}




