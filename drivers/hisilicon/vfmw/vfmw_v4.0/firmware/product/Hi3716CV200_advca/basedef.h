
/***********************************************************************
*
* Copyright (c) 2006 HUAWEI - All Rights Reserved
*
* File: $basedef.h$
* Date: $2006/11/30$
* Revision: $v1.0$
* Purpose: basic definition common in the whole project
*
*
* Change History:
*
* Date             Author            Change
* ====             ======            ======
* 2006/11/30       z56361            Original
*
* Dependencies:
*
************************************************************************/

#ifndef __BASETYPE_DEFS__
#define __BASETYPE_DEFS__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENV_ARMLINUX_KERNEL
    //#include "li_klib.h"
#else
    #include "stdio.h"
    #include "stdlib.h"
    #include "memory.h"
#endif

#include "sysconfig.h"
#include "vfmw_osal.h"

/*************************************************************************
 *	This software supports the following platform. Platform is defined in
 *  the according compile environment( eg. use '-D' in the makefile ).
 *      ENV_WIN32               --- WIN32 debug
 *      ENV_ARMLINUX            --- FGPA test(LINUX)
 *      ENV_ARMLINUX_KERNEL     --- final version
 *      ENV_VXWORKS             --- FGPA test(VXWORKS)
 *************************************************************************/


/*************************************************************************
 *	compile switches.
 *************************************************************************/
#if 0
#ifndef BYPASS_FOD
#define BYPASS_FOD               //FOD����
#endif
#endif

#if defined(ENV_WIN32)

#elif defined(ENV_ARMLINUX_KERNEL)
#define USE_DECODE_THREAD       /* ARMLINUX�ں�̬һ��Ҫ���������߳� */
#endif

#define H264_ENABLE
#define MVC_ENABLE
#define MPEG2_ENABLE
#define AVS_ENABLE
#define MPEG4_ENABLE
#define REAL8_ENABLE
#define REAL9_ENABLE
#define VC1_ENABLE
#define DIVX3_ENABLE
#define VP6_ENABLE
#define VP8_ENABLE

typedef UINT32    PHYADDR;

/*************************************************************************
 *	macro
 *************************************************************************/

extern SINT32  g_DbgMemPhy;
extern SINT32  *g_pDbgMemVir;

#ifndef VFMW_CFG_CAP_HD_SIMPLE
#define REC_POS(Data)  DBG_AddTrace( (SINT8*)__FUNCTION__, __LINE__, (SINT32)Data)
#else
#define REC_POS(Data)
#endif

/* MAX()/MIN(): �����С */
#define MAX(a, b)         (( (a) < (b) ) ?  (b) : (a))
#define MIN(a, b)         (( (a) > (b) ) ?  (b) : (a))
#define ABS(x)            (( (x) <  0  ) ? -(x) : (x))
#define SIGN(a)           (( (a)<0 ) ? (-1) : (1))
#define MEDIAN(a, b, c)   ((a) + (b) + (c) - MIN((a), MIN((b), (c))) - MAX((a), MAX((b), (c))))

#define CLIP1(high, x)             (MAX( MIN((x), high), 0))
#define CLIP3(low, high, x)        (MAX( MIN((x), high), low))
#define CLIP255(x)                 (MAX( MIN((x), 255), 0))

#ifdef ENV_ARMLINUX_KERNEL
#ifndef  HI_ADVCA_FUNCTION_RELEASE
#define pos()  dprint(PRN_ALWS,"%s: L%d\n", __FUNCTION__, __LINE__ )
#else
#define pos()    
#endif
#elif defined(ENV_ARMLINUX)
#define pos()  printf("%s: L%d\n", __FUNCTION__, __LINE__ )
#else
#define pos()  printf("%s: L%d\n", __FUNCTION__, __LINE__ )
#endif

/* =============== ����ƽ̨����ĺ궨�� ================ */
#if !defined(ENV_WIN32) && !defined(ENV_ARMLINUX) && !defined(ENV_ARMLINUX_KERNEL) && !defined(ENV_VXWORKS) && !defined(ENV_ADS)
    #define ENV_ADS	//��ΪADS��DEGUGģʽ��֧��Ԥ���壬����������Ҫ������һ�δ���
#endif


#if defined( ENV_WIN32 )   /* WIN32�µ��Ի��� */
    #define MALLOC( len, align )			malloc( (len) )
    #define FREE( p )						free((p))
	#define RET_MEM_ERR						NULL
	// semaphore
	#define  SEM							SINT32
	#if 0
	#define  SEM_INIT( pSem, val )          while(0)
	#define  SEM_DOWN( pSem, timeout )	    1
	#define  SEM_UP(pSem)			        1
	#define  SEM_TIMEOUT					0
	#endif
    // other
    #define  ASSERT(arg)                    //assert(arg)
    //typedef  HANDLE                         TASK_HANDLE;
    #define  TASK_HANDLE                    HANDLE

#elif defined( ENV_ARMLINUX )   /* FGPA�ϵ�LINUX���Ի��� */
    #define MALLOC( len, align )			malloc( (len) )
    #define FREE( p )						free( (p) )
	#define RET_MEM_ERR						NULL
	// semaphore
	#define  SEM							SINT32
	#if 0
	#define  SEM_INIT( pSem, val )          ((*(pSem))=val)
	#define  SEM_DOWN( pSem, timeout )      ((*(pSem))--)
	#define  SEM_UP(pSem)			        ((*(pSem))++)
	#define  SEM_TIMEOUT					0
	#endif
    // other
    #define  ASSERT(arg)                        ;
    #define  TASK_HANDLE                      pthread_t

#elif defined( ENV_ARMLINUX_KERNEL ) /* �ں�̬�汾�����ս��������� */
    #define SLEEP( ms )                      \
    do{                                      \
        msleep(ms);                          \
	}while(0)

    #define MALLOC( len, align )			klib_malloc( len )
    #define FREE( p )				        kfree( p )
	#define RET_MEM_ERR						NULL
	// LINUX��semaphoreû�г�ʱ���ػ��ƣ�����ֻ�ǽ���semaphore�����֣�ʵ�����õ�����Ϣ����
	#define  SEM						    KLIB_SEM
	#if 0
	extern SINT32 SEM_INIT(SEM *pSem, SINT32 val);
	extern SINT32 SEM_DOWN(SEM *pSem, SINT32 Time);
	extern SINT32 SEM_UP(  SEM *pSem);
	#define  SEM_TIMEOUT					0
	#endif
    // other
	#define  ASSERT(arg)
    #define TASK_HANDLE                      struct task_struct;

#elif defined( ENV_VXWORKS )   /* ��VXWORKS���� */
    #define SLEEP( ms )                      \
    do{                                      \
        taskDelay(ms);                       \
	}while(0)
	#define MALLOC( len, align )			cacheDmaMalloc( (len) )
    #define FREE( p )                       free((p))
	#define RET_MEM_ERR						NULL
	// semaphore
	#define  SEM							SEM_ID
	#if 0
	#define  SEM_INIT( pSem, val )
	                                                            /*\
	                                                            *pSem=semBCreate (SEM_Q_PRIORITY, SEM_EMPTY );\
	                                                            if (0 != val)\
	                                                                {\
	                                                                    semGive(pSem);\
	                                                                }*/
	#define  SEM_DOWN( pSem, timeout )	    semTake(*pSem, timeout)
	#define  SEM_UP(pSem)			        semGive(*pSem)
	#define  SEM_TIMEOUT					-1
	#endif
    // other
    #define  ASSERT(arg)                    assert(arg)
    #define TASK_HANDLE                     UINT32;

#elif defined( ENV_ADS )   /* ��ADS���� */
	#define MALLOC( len, align )			malloc( (len) )
    #define FREE( p )						free((p))
	#define RET_MEM_ERR						NULL
	// semaphore
	#define  SEM							SINT32
	#if 0
	#define  SEM_INIT( pSem, val )          while(0)
	#define  SEM_DOWN( pSem, timeout )	    1
	#define  SEM_UP(pSem)			        1
	#define  SEM_TIMEOUT					0
	#endif
    // other
    #define  ASSERT(arg)                    assert(arg)
    #define  TASK_HANDLE                     UINT32;

#endif

#if 0
/* DPRINT(): ��Ļ��ӡ */
#ifdef SCREEN_PRINT_ON
    #if defined(ENV_ARMLINUX_KERNEL)
        #define DPRINT printk
    #else
        #define DPRINT printf
    #endif
#else
    #if defined(ENV_ARMLINUX_KERNEL)
        #define DPRINT(...) do{}while(0)
    #else
        #define DPRINT
    #endif

#endif

#define PRINT_FUCTION_FLOW
    #ifdef PRINT_FUCTION_FLOW
    #define DPRINT_FUN(arg)                                    \
    do											               \
    {												           \
        DPRINT("Function:%s, Line:%d, Mark %d\n", __FUNCTION__,__LINE__,arg); \
    }while(0)
#else
    #define DPRINT_FUN(arg)
#endif


/* MARKER(): ��������е�marker_bit */
#define MARKER( pBs )										    \
	if( !BsGet( pBs, 1 ) ){										\
	    DPRINT( "fatal stream error --- marker_bit=0\n" );		\
	    return( ERR_STRM_DAT );									\
    }
#endif

/* _LOG(): ��LOG�ļ��д�ӡ��Ϣ */
#ifdef  LOG_PRINT_ON
    #if defined( ENV_ARMLINUX_KERNEL )  /* ARMLINUX �ں�̬�汾 */
        #define _LOG(...)                                       \
        if( 0 != g_LogEnable )                                  \
        {                                                       \
            UINT8 logchar[500];                                 \
            snprintf(logchar, 500, __VA_ARGS__ );               \
            klib_fwrite(logchar, strlen(logchar)+1, g_fpLog);   \
        }
	    //�ں�̬����ʱ��֧�ִ������Ĺ��ܣ���Ϊ�������������ļ� �ķ���
        #define _FWRITE

    #elif defined( ENV_ARMLINUX )       /* ARMLINUX �û�̬�汾 */
        #define _LOG(...)                                       \
	    if( 0 != g_LogEnable )                                  \
	    {                                                       \
		    fprintf( g_fpLog, __VA_ARGS__ );                    \
		    fflush( g_fpLog );	                                \
        }
	    //�û�̬����ʱ��֧�ִ������Ĺ��ܣ���Ϊ�������������ļ� �ķ���
        #define _FWRITE(SrcBuffer,WriteSize)

    #elif defined( ENV_WIN32 )   /* WIN32�汾�²��ú꣬�ú���ʵ�ֿɱ��������VXWORKS�����ٶ��� */
        SINT32 _LOG( const char *format, ...  );
        #define _FWRITE(SrcBuffer,WriteSize)                    \
        if( 0 != g_StreamEnable )                               \
        {                                                       \
            fwrite(SrcBuffer, WriteSize, sizeof(UINT8), g_fpStreamLog); \
            fflush( g_fpStreamLog );	                                \
        }


    #elif  defined(ENV_ADS)
        #define _LOG(...)                                       \
        if( 0 != g_LogEnable )                                  \
        {                                                       \
            fprintf( g_fpLog, __VA_ARGS__ );                    \
            fflush( g_fpLog );	                                \
        }

        #define _FWRITE(SrcBuffer,WriteSize)                    \
        if( 0 != g_StreamEnable )                               \
        {                                                       \
            fwrite(SrcBuffer, WriteSize, sizeof(UINT8), g_fpStreamLog); \
            fflush( g_fpStreamLog );	                                \
        }
    #endif

#else
    #if !defined(ENV_WIN32)
        #define _LOG(...)        \
            do{ } while(0)
    #endif

    #define _FWRITE(SrcBuffer,WriteSize)
#endif

#if defined(ENV_VXWORKS)
    #ifdef _LOG
        #undef _LOG
    #endif

    #define _LOG printf
#endif

#if 0
/* DTRACE: ��ӡ������Ϣ */
#ifdef ENV_ARMLINUX_KERNEL
    #define DTRACE HI_TRACE_VDEC

#else
    #define DTRACE DPRINT
#endif
#endif
#if 0
#ifdef ENV_ARMLINUX
#ifndef ENV_EDA
#define  MEMMAP(addr, len)             memmap(addr, len)
#else
#define  MEMMAP(addr, len)             MALLOC( len, 4)
#endif
#else
#define  MEMMAP(addr, len)             MALLOC( len, 4)
#endif
#endif
#ifdef __cplusplus
}
#endif

#endif	//end of "#ifndef __BASETYPE_DEFS__"

