
/******************************************************************************

  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

******************************************************************************
    �� �� ��   : vfmw_osal.h
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

#ifndef __VFMW_OSAL_HEADER__
#define  __VFMW_OSAL_HEADER__

//#ifdef __LINUX_2_6_31__
#include "vfmw.h"
//#include "hi_type_cbb.h"
//#else
//#include "hi_type.h"
//#endif

#include "hi_type.h"
#include "hi_module.h"
#include "hi_drv_mmz.h"

//#include "hi_common_id.h"
//#include "common_mem_drv.h"
//#include "common_mem.h"
//#include "hi_common_mem.h"


#include <linux/kthread.h>
#include <linux/timer.h>   /* time test, z56361 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>

#include <linux/wait.h>
#include <linux/syscalls.h>
#include <linux/sched.h>

#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
//#include <linux/smp_lock.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <asm/system.h>
//#ifdef __LINUX_2_6_31__
#include <linux/semaphore.h>
//#include "hi_mmz.h"
//#include "drv_mmz.h"
#include "hi_drv_mmz.h"
/*======================================================================*/
/*                            ��������                                  */
/*======================================================================*/
/*
typedef  unsigned long long UINT64;
typedef  long long			SINT64;
typedef  unsigned int		UINT32;
typedef  int				SINT32;
typedef  unsigned short		UINT16;
typedef  short				SINT16;
typedef  char				SINT8;
typedef  const void        CONSTVOID;
typedef  unsigned          USIGN;
*/
//#ifndef __LINUX_2_6_31__
//typedef  unsigned char     UINT8;
//typedef  void              VOID;
//typedef  unsigned long     ULONG;
//#endif

#define OSAL_OK     0
#define OSAL_ERR   -1

#define DIV_NS_TO_MS  1000000
#define DIV_NS_TO_US  1000

/*======================================================================*/
/*                            ���ݽṹ��ö��                            */
/*======================================================================*/
typedef struct hiOSAL_MEM_S
{
    SINT32     PhyAddr;
	VOID       *VirAddr;
	SINT32     Length;
} OSAL_MEM_S;

typedef enum hiFILE_OP_E
{
    OSAL_FREAD,
	OSAL_FWRITE,
	OSAL_FREAD_FWRITE
} OSAL_FILE_OP_E;

typedef enum hiFILE_TYPE_E
{
    OSAL_FILE_ASCII,
	OSAL_FILE_BIN,
} OSAL_FILE_TYPE_E;

typedef enum hiFILE_SEEK_START_E
{
    OSAL_SEEK_SET,
	OSAL_SEEK_CUR,
	OSAL_SEEK_END
} FILE_SEEK_START_E;


typedef struct hiKERN_EVENT_S
{
	wait_queue_head_t   queue_head;
	SINT32              flag;    
} KERN_EVENT_S;

typedef struct hiKERN_IRQ_LOCK_S
{
    spinlock_t     irq_lock;
    unsigned long  irq_lockflags;
    int            isInit;
} KERN_IRQ_LOCK_S;

/*======================================================================*/
/*                            �������ͳ���                              */
/*======================================================================*/
/* ���� ---  ͨ���������߳�.   ��������:  ���������٣�����  */
typedef  struct task_struct*    OSAL_TASK;

/* �¼� --- ͨ�Ż�Э�����ƣ�֧�ֵȴ���ʱ����.  ��������:  ��ʼ�����������ȴ�  */
typedef  KERN_EVENT_S           OSAL_EVENT;

/* ������ --- �߳�֮����ٽ�����������.  ��������:  ����������������  */
typedef  KERN_EVENT_S           OSAL_TASK_MUTEX;

/* ������ --- �߳����жϷ������֮����ٽ�������.  ��������:  ����������������  */
typedef  UINT32                 OSAL_IRQ_LOCK;

typedef KERN_IRQ_LOCK_S         OSAL_IRQ_SPIN_LOCK;

/* �ļ� */
typedef  struct file           OSAL_FILE;    
typedef  struct file   FILE;
typedef struct{
	wait_queue_head_t   queue_head;
	int                 flag;
}KLIB_SEM;
typedef struct semaphore OSAL_SEMA;
/*======================================================================*/
/*                           ��������                                   */
/*======================================================================*/

/************************************************************************/
/* ����(���뾫��)                                                       */
/************************************************************************/
#if 1
#define OSAL_MSLEEP(nMs)    msleep(nMs)
#else
#define OSAL_MSLEEP(nMs) \
do{\
    msleep(nMs);\
    dprint(PRN_ALWS,"sleep in %s:%d\n", __FUNCTION__, __LINE__);\
}while(0)
#endif
/************************************************************************/
/* ��ȡϵͳʱ��                                                         */
/************************************************************************/
UINT32 OSAL_GetTimeInMs(VOID);
UINT32 OSAL_GetTimeInUs(VOID);

VOID OSAL_AcrtUsSleep(UINT32 us);

/************************************************************************/
/* ���������ڴ棨���ܷ�����������                                       */
/************************************************************************/
VOID *OSAL_AllocVirMem(SINT32 Size);

/************************************************************************/
/* �ͷ������ڴ棨���ܷ�����������                                       */
/************************************************************************/
VOID OSAL_FreeVirMem(VOID *p);

/************************************************************************/
/* ��������                                                             */
/************************************************************************/
SINT32 OSAL_CreateTask( OSAL_TASK *pTask, SINT8 TaskName[], VOID *pTaskFunction );
/************************************************************************/
/* ��������                                                             */
/************************************************************************/
SINT32 OSAL_WakeupTask( OSAL_TASK *pTask );
/************************************************************************/
/* ��������                                                             */
/************************************************************************/
SINT32 OSAL_DeleteTask(OSAL_TASK *pTask);


/************************************************************************/
/* ��ʼ���¼�                                                           */
/************************************************************************/
SINT32 OSAL_InitEvent( OSAL_EVENT *pEvent, SINT32 InitVal ); 
/************************************************************************/
/* �����¼�                                                             */
/************************************************************************/
SINT32 OSAL_GiveEvent( OSAL_EVENT *pEvent ); 
/************************************************************************/
/* �ȴ��¼�                                                             */
/* �¼���������OSAL_OK����ʱ����OSAL_ERR                                */
/************************************************************************/
SINT32 OSAL_WaitEvent( OSAL_EVENT *pEvent, SINT32 msWaitTime ); 


/************************************************************************/
/* ��ʼ���̻߳�����                                                     */
/************************************************************************/
SINT32 OSAL_InitTaskMutex( OSAL_TASK_MUTEX *pTaskMutex );
/************************************************************************/
/* �̻߳������                                                         */
/************************************************************************/
SINT32 OSAL_LockTaskMutex( OSAL_TASK_MUTEX *pTaskMutex );
/************************************************************************/
/* �̻߳������                                                         */
/************************************************************************/
SINT32 OSAL_UnlockTaskMutex( OSAL_TASK_MUTEX *pTaskMutex );


/************************************************************************/
/* ��ʼ���жϻ�����                                                     */
/************************************************************************/
SINT32 OSAL_InitIntrMutex( OSAL_IRQ_LOCK *pIntrMutex );
/************************************************************************/
/* ���ж�                                                               */
/************************************************************************/
SINT32 OSAL_LockIRQ( OSAL_IRQ_LOCK *pIntrMutex );
/************************************************************************/
/* ���ж�                                                               */
/************************************************************************/
SINT32 OSAL_UnLockIRQ( OSAL_IRQ_LOCK *pIntrMutex );

/************************************************************************/
/* ��ʼ��������                                                         */
/************************************************************************/
SINT32 OSAL_InitSpinLock( OSAL_IRQ_SPIN_LOCK *pIntrMutex );
/************************************************************************/
/* �������                                                             */
/************************************************************************/
SINT32 OSAL_SpinLock( OSAL_IRQ_SPIN_LOCK *pIntrMutex );
/************************************************************************/
/* �������                                                             */
/************************************************************************/
SINT32 OSAL_SpinUnLock( OSAL_IRQ_SPIN_LOCK *pIntrMutex );

/************************************************************************/
/* ����ʼ��                                                             */
/************************************************************************/
VOID OSAL_SpinLockIRQInit( OSAL_IRQ_SPIN_LOCK *pIntrMutex );

/************************************************************************/
/* �жϻ������(���ж��Ҽ���)                                           */
/************************************************************************/
SINT32 OSAL_SpinLockIRQ( OSAL_IRQ_SPIN_LOCK *pIntrMutex );
/************************************************************************/
/* �жϻ������(���ж���ȥ��)                                           */
/************************************************************************/
SINT32 OSAL_SpinUnLockIRQ( OSAL_IRQ_SPIN_LOCK *pIntrMutex );

/************************************************************************/
/* �����ڴ�                                                             */
/************************************************************************/
SINT32 OSAL_AllocMemory( SINT32 ExpectPhyAddr, SINT32 ExpectSize, OSAL_MEM_S *pOsalMem );
/************************************************************************/
/* �ͷ��ڴ�                                                             */
/************************************************************************/
SINT32 OSAL_ReleaseMemory( OSAL_MEM_S *pMemRet );
/************************************************************************/
/* ӳ��Ĵ��������ַ                                                   */
/************************************************************************/
SINT32 OSAL_MapRegisterAddr( SINT32 RegPhyAddr, SINT32 Range, OSAL_MEM_S *pOsalMem );
/************************************************************************/
/* ȥӳ��Ĵ��������ַ                                                 */
/************************************************************************/
SINT32 OSAL_UnmapRegisterAddr( OSAL_MEM_S *pOsalMem );

/************************************************************************/
/* ���ļ�                                                             */
/************************************************************************/
OSAL_FILE *OSAL_OpenFile( SINT8 *pFileName, OSAL_FILE_TYPE_E eType, OSAL_FILE_OP_E eFileOP);

/************************************************************************/
/* �ر��ļ�                                                             */
/************************************************************************/
VOID OSAL_CloseFile( OSAL_FILE *pFile );

/************************************************************************/
/* �ļ�read                                                             */
/************************************************************************/
SINT32 OSAL_ReadFile( VOID *pBuf, SINT32 Size, OSAL_FILE *pFile );

/************************************************************************/
/* �ļ�seek                                                             */
/************************************************************************/
SINT32 OSAL_SeekFile( OSAL_FILE *pFile, SINT32 Offset, FILE_SEEK_START_E eStartPoint );

/************************************************************************/
/* �ļ�tell position                                                    */
/************************************************************************/
SINT32 OSAL_TellFilePos( OSAL_FILE *pFile );

/************************************************************************/
/* file: open/close/write                                               */
/************************************************************************/
struct file *klib_fopen(const char *filename, int flags, int mode);
void klib_fclose(struct file *filp);
int klib_fread(char *buf, unsigned int len, struct file *filp);
int klib_fwrite(char *buf, int len, struct file *filp);

/************************************************************************/
/* memory: alloc/free/map/unmap                                         */
/************************************************************************/
//unsigned int klib_phymalloc(const char *string, unsigned int len, unsigned int align);

//void klib_phyfree(unsigned int phyaddr);
//unsigned char *klib_mmap(unsigned int phyaddr, unsigned int len);
//unsigned char *klib_mmap_cache(unsigned int phyaddr, unsigned int len);
//void klib_munmap(unsigned char *p );
//void klib_flush_cache(void *ptr, unsigned int len);
void klib_flush_cache(void *ptr, unsigned int phy_addr, unsigned int len);

SINT32 KernelMemMalloc(UINT8 * MemName, UINT32 len, UINT32 align, UINT32 IsCached, MEM_DESC_S *pMemDesc);
SINT32 KernelMemFree(MEM_DESC_S *pMemDesc);
UINT8 *KernelRegisterMap(UINT32 PhyAddr);
VOID KernelRegisterUnMap(UINT8 *VirAddr);
UINT8 *KernelMmap(UINT32 phyaddr, UINT32 len);
UINT8 *KernelMmapCache(UINT32 phyaddr, UINT32 len);
VOID KernelMunmap(UINT8 *p );
VOID KernelFlushCache(VOID *ptr, UINT32 phy_addr, UINT32 len);
SINT32 OSAL_DOWN_INTERRUPTIBLE(VOID);
VOID OSAL_UP(VOID);
VOID OSAL_SEMA_INTIT(VOID);
char * OSAL_KMALLOC(SINT32 s32Size);
VOID  OSAL_KFREE(SINT32 s32Addr);
#endif


