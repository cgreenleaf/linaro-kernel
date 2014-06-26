 
/******************************************************************************

  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

******************************************************************************
    �� �� ��   : vfmw_osal.c
    �� �� ��   : ����
    ��    ��   : 
    ��������   : 
    ����޸�   :
    ��������   : Ϊvfmw���ƵĲ���ϵͳ����ģ��
                 

    �޸���ʷ   :
    1.��    �� : 2009-05-12
    ��    ��   : 
    �޸�����   : 

******************************************************************************/

#include "vfmw_osal.h"
#include "public.h"

//#include "hi_common_id.h"
//#include "common_mem_drv.h"
//#include "drv_mem_base.h"
//#include "common_sys.h"
//#include "mpi_sys.h"
#include "hi_module.h"
#include "hi_drv_mem.h"
#include "hi_drv_mmz.h"

#include <linux/hrtimer.h>
#include <asm/cacheflush.h>

OSAL_SEMA g_stSem;

VOID OSAL_AcrtUsSleep(UINT32 us)
{
	int ret;
	ktime_t expires;
    expires = ktime_add_ns(ktime_get(), us*1000);
	set_current_state(TASK_UNINTERRUPTIBLE);
    ret = schedule_hrtimeout(&expires, HRTIMER_MODE_ABS);
}


#if 1
/************************************************************************/
/* OSAL_GetTimeInMs():  ��ȡϵͳʱ��                                    */
/************************************************************************/
UINT32 OSAL_GetTimeInMs(VOID)
{
	UINT32 CurrMs = 0;
	
    struct timeval CurrentTime;      
    do_gettimeofday(&CurrentTime);
    CurrMs = (UINT32)(CurrentTime.tv_sec*1000 + CurrentTime.tv_usec/1000);
	
	return CurrMs;
}

UINT32 OSAL_GetTimeInUs(VOID)
{
	UINT32 CurrUs = 0;
	
    struct timeval CurrentTime;      
    do_gettimeofday(&CurrentTime);
    CurrUs = (UINT32)(CurrentTime.tv_sec*1000000 + CurrentTime.tv_usec);
	
	return CurrUs;
}
#else
UINT32 OSAL_GetTimeInMs(VOID)
{
    UINT64 u64TimeNow;
    UINT64 ns;
    UINT32 CurrMs = 0;

    u64TimeNow = sched_clock();

    do_div(u64TimeNow, DIV_NS_TO_MS);
    CurrMs = (UINT32)u64TimeNow;

    return CurrMs;
}

UINT32 OSAL_GetTimeInUs(VOID)
{
    UINT64 u64TimeNow;
    UINT64 ns;
    UINT32 CurrUs = 0;

    u64TimeNow = sched_clock();

    do_div(u64TimeNow, DIV_NS_TO_US);
    CurrUs = (UINT32)u64TimeNow;

    return CurrUs;
}
#endif


/************************************************************************/
/* ���������ڴ棨���ܷ�����������                                       */
/************************************************************************/
VOID *OSAL_AllocVirMem(SINT32 Size)
{
//	return vmalloc(Size);
	return HI_VMALLOC(HI_ID_VFMW, Size);

}

/************************************************************************/
/* �ͷ������ڴ棨���ܷ�����������                                       */
/************************************************************************/
VOID OSAL_FreeVirMem(VOID *p)
{

//	if(p)vfree(p);
    if(p)
    {
        HI_VFREE(HI_ID_VFMW, p);
    }
}

/*  ��������                                                            */
/************************************************************************/
SINT32 OSAL_CreateTask( OSAL_TASK *pTask, SINT8 TaskName[], VOID *pTaskFunction )
{
    *pTask = kthread_create(pTaskFunction, (VOID *)NULL, TaskName);
    if( NULL == (*pTask) ) 
    {
        dprint(PRN_FATAL, "can not create thread!\n");
        return( VF_ERR_SYS );
    }
    wake_up_process(*pTask);	
    return OSAL_OK;
}

/************************************************************************/
/* ��������                                                             */
/************************************************************************/
SINT32 OSAL_WakeupTask( OSAL_TASK *pTask )
{
    return OSAL_OK;
}

/************************************************************************/
/* ��������                                                             */
/************************************************************************/
SINT32 OSAL_DeleteTask(OSAL_TASK *pTask)
{
    return OSAL_OK;
}

/************************************************************************/
/* ��ʼ���¼�                                                           */
/************************************************************************/
SINT32 OSAL_InitEvent( OSAL_EVENT *pEvent, SINT32 InitVal )
{
	pEvent->flag = InitVal;
	init_waitqueue_head( &(pEvent->queue_head) );	
	return OSAL_OK;
}

/************************************************************************/
/* �����¼�                                                             */
/************************************************************************/
SINT32 OSAL_GiveEvent( OSAL_EVENT *pEvent )
{
	pEvent->flag = 1;
	wake_up_interruptible ( &(pEvent->queue_head) );
	
	return OSAL_ERR;
}

/************************************************************************/
/* �ȴ��¼�                                                             */
/* �¼���������OSAL_OK����ʱ����OSAL_ERR ��condition������������ȴ�    */
/************************************************************************/
SINT32 OSAL_WaitEvent( OSAL_EVENT *pEvent, SINT32 msWaitTime )
{
	SINT32 l_ret;

    //l_ret = wait_event_interruptible_timeout( pEvent->queue_head, (pEvent->flag != 0), ((msWaitTime*10+50)/(msWaitTime)) );
    l_ret = wait_event_interruptible_timeout( pEvent->queue_head, (pEvent->flag != 0), msWaitTime/10 );

    pEvent->flag = 0;//(pEvent->flag>0)? (pEvent->flag-1): 0;

	return (l_ret==0)? OSAL_OK: OSAL_ERR;
}


/************************************************************************/
/* ��ʼ���̻߳�����                                                     */
/************************************************************************/
SINT32 OSAL_InitTaskMutex( OSAL_TASK_MUTEX *pTaskMutex )
{
    return OSAL_InitEvent(pTaskMutex, 1);
}
/************************************************************************/
/* �̻߳������                                                         */
/************************************************************************/
SINT32 OSAL_LockTaskMutex( OSAL_TASK_MUTEX *pTaskMutex )
{
	return OSAL_WaitEvent(pTaskMutex, 1000000);
}
/************************************************************************/
/* �̻߳������                                                         */
/************************************************************************/
SINT32 OSAL_UnlockTaskMutex( OSAL_TASK_MUTEX *pTaskMutex )
{
	return OSAL_GiveEvent(pTaskMutex);
}


/************************************************************************/
/* ��ʼ���жϻ�����                                                     */
/************************************************************************/
SINT32 OSAL_InitIntrMutex( OSAL_IRQ_LOCK *pIntrMutex )
{
    return OSAL_OK;
}

extern SINT32  *g_pDbgMemVir;
/************************************************************************/
/* �жϻ������(���ж�)                                                 */
/************************************************************************/
SINT32 OSAL_LockIRQ( OSAL_IRQ_LOCK *pIntrMutex )
{
        unsigned long aa;
	//local_irq_save((unsigned long)(*pIntrMutex));
	local_irq_save(aa);
	*(unsigned long *)pIntrMutex = aa;
	return OSAL_OK;
}

/************************************************************************/
/* �жϻ������(���ж�)                                                 */
/************************************************************************/
SINT32 OSAL_UnLockIRQ( OSAL_IRQ_LOCK *pIntrMutex )
{
	local_irq_restore((unsigned long)(*pIntrMutex));
	return OSAL_OK;
}

/************************************************************************/
/* ��ʼ��������                                                         */
/************************************************************************/
SINT32 OSAL_InitSpinLock( OSAL_IRQ_SPIN_LOCK *pIntrMutex )
{
    spin_lock_init(&pIntrMutex->irq_lock);
    
    return OSAL_OK;
}
/************************************************************************/
/* �������                                                             */
/************************************************************************/
SINT32 OSAL_SpinLock( OSAL_IRQ_SPIN_LOCK *pIntrMutex )
{
    if(pIntrMutex->isInit == 0)
	{
        spin_lock_init(&pIntrMutex->irq_lock);  
        pIntrMutex->isInit = 1;
    }
    spin_lock(&pIntrMutex->irq_lock);

	return OSAL_OK;
}

/************************************************************************/
/* �������                                                             */
/************************************************************************/
SINT32 OSAL_SpinUnLock( OSAL_IRQ_SPIN_LOCK *pIntrMutex )
{
	spin_unlock(&pIntrMutex->irq_lock);

	return OSAL_OK;
}

/************************************************************************/
/* ����ʼ��                                                             */
/************************************************************************/
VOID OSAL_SpinLockIRQInit( OSAL_IRQ_SPIN_LOCK *pIntrMutex )
{
    spin_lock_init(&pIntrMutex->irq_lock);  
    pIntrMutex->isInit = 1;

	return;
}


/************************************************************************/
/* �жϻ������(���ж��Ҽ���)                                           */
/************************************************************************/
SINT32 OSAL_SpinLockIRQ( OSAL_IRQ_SPIN_LOCK *pIntrMutex )
{
    if(pIntrMutex->isInit == 0)
	{
        spin_lock_init(&pIntrMutex->irq_lock);  
        pIntrMutex->isInit = 1;
    }
    spin_lock_irqsave(&pIntrMutex->irq_lock, pIntrMutex->irq_lockflags);

	return OSAL_OK;
}

/************************************************************************/
/* �жϻ������(���ж���ȥ��)                                           */
/************************************************************************/
SINT32 OSAL_SpinUnLockIRQ( OSAL_IRQ_SPIN_LOCK *pIntrMutex )
{
	spin_unlock_irqrestore(&pIntrMutex->irq_lock, pIntrMutex->irq_lockflags);

	return OSAL_OK;
}

/************************************************************************/
/* �����ڴ�                                                             */
/************************************************************************/
SINT32 OSAL_AllocMemory( SINT32 ExpectPhyAddr, SINT32 ExpectSize, OSAL_MEM_S *pOsalMem )
{

    return OSAL_OK;
}
/************************************************************************/
/* �ͷ��ڴ�                                                             */
/************************************************************************/
SINT32 OSAL_ReleaseMemory( OSAL_MEM_S *pOsalMem )
{
    return OSAL_OK;
}
/************************************************************************/
/* ӳ��Ĵ��������ַ                                                   */
/************************************************************************/
SINT32 OSAL_MapRegisterAddr( SINT32 RegPhyAddr, SINT32 Range, OSAL_MEM_S *pOsalMem )
{
    return OSAL_OK;
}
/************************************************************************/
/* ȥӳ��Ĵ��������ַ                                                 */
/************************************************************************/
SINT32 OSAL_UnmapRegisterAddr( OSAL_MEM_S *pOsalMem )
{
    return OSAL_OK;
}

/************************************************************************/
/* ���ļ�                                                             */
/************************************************************************/
OSAL_FILE *OSAL_OpenFile( SINT8 *pFileName, OSAL_FILE_TYPE_E eType, OSAL_FILE_OP_E eFileOP)
{
    struct file *filp;
    SINT32 flags, mode;

    if( OSAL_FREAD == eFileOP )
    {
        flags = O_RDONLY;
	}
	else if ( OSAL_FWRITE == eFileOP )
	{
        flags = O_WRONLY|O_CREAT;
	}
	else
	{
        flags = O_RDWR|O_CREAT;
	}

    mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	filp = filp_open(pFileName, flags, mode);
    return (IS_ERR(filp)) ? NULL : filp;	
}

/************************************************************************/
/* �ر��ļ�                                                             */
/************************************************************************/
VOID OSAL_CloseFile( OSAL_FILE *pFile )
{
    if (pFile)
    {
        filp_close(pFile, NULL);
    }
	return;
}

/************************************************************************/
/* �ļ�read                                                             */
/************************************************************************/
SINT32 OSAL_ReadFile( VOID *pBuf, SINT32 Size, OSAL_FILE *pFile )
{
	return 0;
}

/************************************************************************/
/* �ļ�write                                                            */
/************************************************************************/
SINT32 OSAL_WriteFile( VOID *pBuf, SINT32 Size, OSAL_FILE *pFile )
{
#if 0	
	int writelen;
	mm_segment_t oldfs;
	if (filp == NULL)
			return 0;
	if (filp->f_op->write == NULL)
			return 0;
	if (((filp->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
			return 0;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	writelen = filp->f_op->write(filp, buf, len, &filp->f_pos);
	set_fs(oldfs);
	
	return writelen;
#else
    return -1;
#endif
}

/************************************************************************/
/* �ļ�seek                                                             */
/************************************************************************/
SINT32 OSAL_SeekFile( OSAL_FILE *pFile, SINT32 Offset, FILE_SEEK_START_E eStartPoint )
{
    return 0;
}

/************************************************************************/
/* �ļ�tell position                                                    */
/************************************************************************/
SINT32 OSAL_TellFilePos( OSAL_FILE *pFile )
{

    return -1;
}

struct file *klib_fopen(const char *filename, int flags, int mode)
{
        struct file *filp = filp_open(filename, flags, mode);
        return (IS_ERR(filp)) ? NULL : filp;
}

void klib_fclose(struct file *filp)
{
        if (filp)
            filp_close(filp, NULL);
}

int klib_fread(char *buf, unsigned int len, struct file *filp)
{
        int readlen;
        mm_segment_t oldfs;

        if (filp == NULL)
                return -ENOENT;
        if (filp->f_op->read == NULL)
                return -ENOSYS;
        if (((filp->f_flags & O_ACCMODE) & (O_RDONLY | O_RDWR)) == 0)
                return -EACCES;
        oldfs = get_fs();
        set_fs(KERNEL_DS);
        readlen = filp->f_op->read(filp, buf, len, &filp->f_pos);
        set_fs(oldfs);

        return readlen;
}

int klib_fwrite(char *buf, int len, struct file *filp)
{
        int writelen;
        mm_segment_t oldfs;

        if (filp == NULL)
                return -ENOENT;
        if (filp->f_op->write == NULL)
                return -ENOSYS;
        if (((filp->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
                return -EACCES;
        oldfs = get_fs();
        set_fs(KERNEL_DS);
        writelen = filp->f_op->write(filp, buf, len, &filp->f_pos);
        set_fs(oldfs);

        return writelen;
}

#if 0
unsigned int klib_phymalloc(const char *string, unsigned int len, unsigned int align)
{
    mmb_addr_t addr;

    addr = new_mmb(string, len, align, NULL);
    if (MMB_ADDR_INVALID == addr)
    {
        return 0;
    }

	return addr;
}

void klib_phyfree(unsigned int  phyaddr)
{
    delete_mmb(phyaddr);
}

unsigned char *klib_mmap(unsigned int phyaddr, unsigned int len)
{
    return (unsigned char *)remap_mmb (phyaddr);
}

unsigned char *klib_mmap_cache(unsigned int phyaddr, unsigned int len)
{
    return (unsigned char *)remap_mmb_cached((mmb_addr_t)phyaddr);
}

void klib_munmap(unsigned char *p )
{
    unmap_mmb (p);
}
#endif

void klib_flush_cache(void *ptr, unsigned int phy_addr, unsigned int len)
{
    unsigned long flags;
    DEFINE_SPINLOCK(cache_lock);
    
    spin_lock_irqsave(&cache_lock, flags);

    __cpuc_flush_dcache_area((void *)ptr, (size_t)len); // flush l1cache
    outer_flush_range(phy_addr, phy_addr+len); // flush l2cache

    spin_unlock_irqrestore(&cache_lock, flags);
    return;
}

VOID KernelFlushCache(VOID *ptr, UINT32 phy_addr, UINT32 len)
{
    unsigned long flags;
    DEFINE_SPINLOCK(cache_lock);
    
    spin_lock_irqsave(&cache_lock, flags);
    __cpuc_flush_dcache_area((void *)ptr, (size_t)len); // flush l1cache
    outer_flush_range(phy_addr, phy_addr+len); // flush l2cache
    
    spin_unlock_irqrestore(&cache_lock, flags);
    return;
}


SINT32 KernelMemMalloc(UINT8 * MemName, UINT32 len, UINT32 align, UINT32 IsCached, MEM_DESC_S *pMemDesc)
{
//    pMemDesc->PhyAddr = klib_phymalloc( MemName, len, align);
    MMZ_BUFFER_S stMmzBuffer;
    memset(&stMmzBuffer, 0, sizeof(MMZ_BUFFER_S));

    if(HI_SUCCESS != HI_DRV_MMZ_Alloc(MemName, NULL, len, align, &stMmzBuffer))
    {
        memset(pMemDesc, 0, sizeof(MEM_DESC_S));
        return -1;
    }
    
    if( 0 != stMmzBuffer.u32StartPhyAddr )
    {
        if(IsCached == 1)
        {
//            pMemDesc->VirAddr = (UINT8*)klib_mmap_cache( pMemDesc->PhyAddr, len);
            if(HI_SUCCESS != HI_DRV_MMZ_MapCache(&stMmzBuffer))
            {
                memset(pMemDesc, 0, sizeof(MEM_DESC_S));
                return -1;
            }
        }
        else
        {
//            pMemDesc->VirAddr = (UINT8*)klib_mmap( pMemDesc->PhyAddr, len);
            if(HI_SUCCESS != HI_DRV_MMZ_Map(&stMmzBuffer))
            {
                memset(pMemDesc, 0, sizeof(MEM_DESC_S));
                return -1;
            }
        }
    }
    else
    {
    	memset(pMemDesc, 0, sizeof(MEM_DESC_S));
		return -1;
    }

    pMemDesc->PhyAddr = (SINT32)stMmzBuffer.u32StartPhyAddr;
    pMemDesc->VirAddr = (UINT8*)stMmzBuffer.u32StartVirAddr;
    pMemDesc->Length  = (SINT32)stMmzBuffer.u32Size;

    
    return 0;
}

SINT32 KernelMemFree(MEM_DESC_S *pMemDesc)
{
//    klib_munmap(pMemDesc->VirAddr);
//    klib_phyfree(pMemDesc->PhyAddr);
    MMZ_BUFFER_S stMBuf;
    memset(&stMBuf, 0, sizeof(MMZ_BUFFER_S));

    stMBuf.u32StartPhyAddr = (UINT32)pMemDesc->PhyAddr;
    stMBuf.u32StartVirAddr = (UINT32)pMemDesc->VirAddr;
    stMBuf.u32Size         = (UINT32)pMemDesc->Length;

    HI_DRV_MMZ_Unmap(&stMBuf);
    HI_DRV_MMZ_Release(&stMBuf);

    return 0;	
}		

UINT8 *KernelRegisterMap(UINT32 PhyAddr)
{
	return (UINT8*)ioremap_nocache( PhyAddr,0x10000 );
}

VOID KernelRegisterUnMap(UINT8 *VirAddr)
{
    iounmap(VirAddr);
	return;
}

UINT8 *KernelMmap(UINT32 phyaddr, UINT32 len)
{
//    return (UINT8 *)remap_mmb (phyaddr);
    SINT32 s32Ret = 0;
    MMZ_BUFFER_S stMemBuf;
    memset(&stMemBuf, 0, sizeof(MMZ_BUFFER_S));
    stMemBuf.u32StartPhyAddr = phyaddr;

    s32Ret = HI_DRV_MMZ_Map(&stMemBuf);
	if(s32Ret !=HI_SUCCESS)
	{
	    dprint(PRN_FATAL, "vfmw_osal.c, line %d: HI_DRV_MMZ_Map ERR\n", __LINE__);
	}

    return (UINT8 *)(stMemBuf.u32StartVirAddr);

}

UINT8 *KernelMmapCache(UINT32 phyaddr, UINT32 len)
{
//    return (UINT8 *)remap_mmb_cached((mmb_addr_t)phyaddr);
    MMZ_BUFFER_S stMemBuf;
    memset(&stMemBuf, 0, sizeof(MMZ_BUFFER_S));
    stMemBuf.u32StartPhyAddr = phyaddr;

    (VOID)HI_DRV_MMZ_MapCache(&stMemBuf);

    return (UINT8 *)(stMemBuf.u32StartVirAddr);
}


VOID KernelMunmap(UINT8 *p )
{
//    unmap_mmb (p);
    MMZ_BUFFER_S stMemBuf;
    memset(&stMemBuf, 0, sizeof(MMZ_BUFFER_S));
    stMemBuf.u32StartVirAddr = (UINT32)(p);

    HI_DRV_MMZ_Unmap(&stMemBuf);
}

#if 1
HI_S32 SEM_INIT(KLIB_SEM *pSem, HI_S32 val )
{
	pSem->flag = val;
	init_waitqueue_head( &(pSem->queue_head) );
	
    return 0;	
}

HI_S32 SEM_DOWN(KLIB_SEM *pSem, HI_S32 Time )
{
	HI_S32 l_ret;

	l_ret = wait_event_interruptible_timeout( pSem->queue_head, (pSem->flag != 0), ((Time*10+50)/(HZ)) );
	pSem->flag = 0;

	return l_ret;
}

HI_S32 SEM_UP( KLIB_SEM *pSem )
{
	wake_up_interruptible ( &(pSem->queue_head) );
	pSem->flag = 1;

	return 0;
}

VOID OSAL_SEMA_INTIT(VOID)
{
    sema_init(&g_stSem,1);
}

SINT32 OSAL_DOWN_INTERRUPTIBLE(VOID)
{
    HI_S32 s32Ret;
	s32Ret = down_interruptible(&g_stSem);
	return s32Ret;
}
VOID OSAL_UP(VOID)
{
	up(&g_stSem);
}

char * OSAL_KMALLOC(SINT32 s32Size)
{
   return kmalloc(s32Size,GFP_ATOMIC);
}

VOID  OSAL_KFREE(SINT32 s32Addr)
{
    kfree((VOID *)s32Addr);
}
EXPORT_SYMBOL(SEM_INIT);
EXPORT_SYMBOL(SEM_DOWN);
EXPORT_SYMBOL(SEM_UP);
#endif

