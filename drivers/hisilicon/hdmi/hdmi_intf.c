/*  extdrv/interface/hdmi/hi_hdmi.c
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 * History:
 *      19-April-2006 create this file
 *      hi_struct.h
 *      hi_debug.h

 */
#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

//#include "common_dev.h" ͷ�ļ�����
#include "hi_drv_dev.h"
//#include "common_proc.h"
#include "hi_drv_proc.h"


//#include "mpi_priv_hdmi.h"
#include "hi_drv_hdmi.h"
#include "drv_hdmi.h"
#include "si_defstx.h"
#include "si_hdmitx.h"
#include "si_edid.h"
#include "si_phy.h"
#include "si_isr.h"
#include "si_delay.h"
#include "si_cec.h"

//#include "hi_common_id.h"
#include "hi_module.h"
//#include "hi_common_log.h"
#include "hi_debug.h"
//#include "common_module_drv.h"
#include "hi_drv_module.h"
#include "drv_hdmi_ext.h"
#include "si_timer.h"
#include "drv_global.h"

#include "hi_osal.h"
#include "drv_reg_proc.h"
#include "drv_compatibility.h"

#ifdef ANDROID_SUPPORT

#include <linux/switch.h>

struct switch_dev hdmi_tx_sdev =
{     
    .name = "hdmi",  
};

HI_BOOL g_switchOk = HI_FALSE;


#endif


#define HDMI_NAME                      "HI_HDMI"
#if defined (SUPPORT_FPGA)
#include "hdmi_fpga.h"
#endif
HI_S32 DRV_HDMI_ReadPhy(void)
{
    HI_U32 u32Ret;
       
    u32Ret = SI_TX_PHY_GetOutPutEnable();
    
    return u32Ret;
}






extern HI_S32 hdmi_Open(struct inode *inode, struct file *filp);
extern HI_S32 hdmi_Close(struct inode *inode, struct file *filp);
extern HI_S32 hdmi_Ioctl(struct inode *inode, struct file *file,
                           unsigned int cmd, HI_VOID *arg);
extern HI_S32 hdmi_Suspend(PM_BASEDEV_S *pdev, pm_message_t state);
extern HI_S32 hdmi_Resume(PM_BASEDEV_S *pdev);

static HDMI_EXPORT_FUNC_S s_stHdmiExportFuncs = {
    .pfnHdmiInit = HI_DRV_HDMI_Init,
    .pfnHdmiDeinit = HI_DRV_HDMI_Deinit,
    .pfnHdmiOpen  = HI_DRV_HDMI_Open,
    .pfnHdmiClose = HI_DRV_HDMI_Close,
    .pfnHdmiGetPlayStus = HI_DRV_HDMI_PlayStus,
    .pfnHdmiGetAoAttr = HI_DRV_AO_HDMI_GetAttr,
    .pfnHdmiGetSinkCapability = HI_DRV_HDMI_GetSinkCapability,
    .pfnHdmiGetAudioCapability = HI_DRV_HDMI_GetAudioCapability,
    .pfnHdmiAudioChange = HI_DRV_HDMI_AudioChange,
    .pfnHdmiPreFormat = HI_DRV_HDMI_PreFormat,
    .pfnHdmiSetFormat = HI_DRV_HDMI_SetFormat,
    .pfnHdmiDetach = HI_DRV_HDMI_Detach,
    .pfnHdmiAttach = HI_DRV_HDMI_Attach,
    .pfnHdmiResume = hdmi_Resume,
    .pfnHdmiSuspend = hdmi_Suspend,
};

static long  hdmi_Drv_Ioctl(struct file *file,unsigned int cmd, unsigned long arg) 
{
	return (long)HI_DRV_UserCopy(file->f_dentry->d_inode, file, cmd, arg, hdmi_Ioctl);
}

static struct file_operations hdmi_FOPS =
{
    owner   : THIS_MODULE,
    open    : hdmi_Open,
    unlocked_ioctl   : hdmi_Drv_Ioctl,
    release : hdmi_Close,
};

static /*struct*/ PM_BASEOPS_S  hdmi_DRVOPS = {
	.probe        = NULL,
	.remove       = NULL,
	.shutdown     = NULL,
	.prepare      = NULL,
	.complete     = NULL,
	.suspend      = hdmi_Suspend,
	.suspend_late = NULL,
	.resume_early = NULL,
	.resume       = hdmi_Resume,
};

static UMAP_DEVICE_S   g_hdmiRegisterData;

#ifndef HI_ADVCA_FUNCTION_RELEASE

static HI_U8 *g_pDefHDMIMode[] = {"NULL","HDMI","DVI","BUTT"};

static HI_U8 *g_pDispFmtString[] = 
{
    "1080P60", 
    "1080P50", 
    "1080P30", 
    "1080P25",
    "1080P24",        
    "1080i60",        
    "1080i50",        
    "720P60",         
    "720P50",         

    "576P50",         
    "480P60",         

    "PAL",
    "PAL_B",
    "PAL_B1",
    "PAL_D",
    "PAL_D1",
    "PAL_G", 
    "PAL_H",
    "PAL_K",
    "PAL_I",
    "PAL_N",
    "PAL_Nc",
    "PAL_M",
    "PAL_60",

    "NTSC",
    "NTSC_J",
    "NTSC_443",

    "SECAM_SIN",
    "SECAM_COS", 
    "SECAM_L",      
    "SECAM_B",      
    "SECAM_G",      
    "SECAM_D",      
    "SECAM_K",      
    "SECAM_H",      
                                        
    "1440x576i_50",                        
    "1440x480i_60",                        
                                     
    "1080P_24_FP",                         
    "720P_60_FP",                          
    "720P_50_FP",                          
                                         
    "640X480_60",                     
    "800X600_60",                     
    "1024X768_60",                    
    "1280X720_60",                    
    "1280X800_60",                    
    "1280X1024_60",         
    "1360X768_60",          
    "1366X768_60",          
    "1400X1050_60",         
    "1440X900_60",          
    "1440X900_60_RB",       
    "1600X900_60_RB",       
    "1600X1200_60",         
    "1680X1050_60",         
    "1680X1050_60_RB",      
    "1920X1080_60",                   
    "1920X1200_60",                   
    "1920X1440_60",                   
    "2048X1152_60",                   
    "2560X1440_60_RB",                
    "2560X1600_60_RB",                
                        
    "Customer Timing" 
};

static HI_U8 *g_pUnfFmtString[] = 
{
    "1080P60", 
    "1080P50", 
    "1080P30", 
    "1080P25",
    "1080P24",        
    "1080i60",        
    "1080i50",        
    "720P60",         
    "720P50",         

    "576P50",         
    "480P60",         

    "PAL",
    "PAL_N",
    "PAL_Nc",

    "NTSC",
    "NTSC_J",
    "NTSC_PAL_M",

    "SECAM_SIN",
    "SECAM_COS",
  
    "1080P24_FP",
    "720P60_FP",
    "720P50_FP",

    "640x480", 
    "800x600", 
    "1024x768", 
    "1280x720",
    "1280x800",
    "1280x1024",
    "1360x768",
    "1366x768",
    "1400x1050",
    "1440x900",        
    "1440x900_RB",

    "1600x900_RB",
    "1600x1200",
    "1680x1050",
    "1680x1050_RB",
    "1920x1080",
    "1920x1200",
    "1920x1440",
    "2048x1152",
    "2560x1440_RB",
    "2560x1600_RB",
    // this array used in two place,and No butt fmt in first place.
    // so we set last fmt to customerTiming ,for native fmt of proc sink_capability
    "CustomerTiming"
};


static HI_U8 *g_pAudioFmtCode[]= 
{
    "Reserved", "PCM",  "AC3",     "MPEG1", "MP3",   "MPEG2", "AAC",
    "DTS",     "ATRAC", "ONE_BIT", "DDP",   "DTS_HD", "MAT",  "DST",
    "WMA_PRO"
};
#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
static HI_U8 *g_pSampleRate[] = 
{
    "32", "44.1", "48", "88.2","96","176.4","192","BUTT"
};
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
static HI_U8 *g_pSpeaker[] = 
{
    "FL/FR", "LFE", "FC", "RL/RR","RC","FLC/FRC","RLC/RRC",
    "FLW/FRW","FLH/FRH","TC","FCH"
}; 


static HI_U8 *g_pAudInputType[] = { "I2S","SPDIF","HBR","BUTT"};
static HI_U8 *g_pColorSpace[] = {"RGB444","YCbCr422","YCbCr444","Future"};
static HI_U8 *g_pDeepColor[] = {"24bit","30bit","36bit","OFF","BUTT"};

static HI_U8 *g_p3DMode[] = {
    "FPK", 
    "FILED_ALTER",
    "LINE_ALTE",
    "SBS_FULL", 
    "L_DEPTH",
    "L_DEPTH_G_DEPTH",
    "TAB",
    "", //0x07 unknown
    "SBS_HALF",     
};

static HI_U8 *g_pScanInfo[] = {"No Data","OverScan","UnderScan","Future"};
static HI_U8 *g_pPixelRep[] = {"1x(No Repeat)","2x","3x","4x","5x","6x","7x","8x","9x","10x","Reserved"};
#endif

HI_S32 hdmi_GetProcArg(HI_CHAR*  chCmd,HI_CHAR*  chArg,HI_U32 u32ArgIdx)
{
    HI_U32 u32Count;
    HI_U32 u32CmdCount;
    HI_U32 u32LogCount;
    HI_U32 u32NewFlag;
    HI_CHAR chArg1[DEF_FILE_NAMELENGTH] = {0};
    HI_CHAR chArg2[DEF_FILE_NAMELENGTH] = {0};
    u32CmdCount = 0;

    /*���ǰ��Ŀո�*/
    u32Count = 0;
    u32CmdCount = 0;
    u32LogCount = 1;
    u32NewFlag = 0;
    while(chCmd[u32Count] != 0 && chCmd[u32Count] != '\n' )
    {
        if (chCmd[u32Count] != ' ')
        {
            u32NewFlag = 1;
        }
        else
        {
            if(u32NewFlag == 1)
            {
                u32LogCount++;
                u32CmdCount= 0;
                u32NewFlag = 0;
            }
        }
        
        if (u32NewFlag == 1)
        {
            switch(u32LogCount)
            {
                case 1:
                    chArg1[u32CmdCount] = chCmd[u32Count];
                    u32CmdCount++;
                    break;
                case 2:
                    chArg2[u32CmdCount] = chCmd[u32Count];
                    u32CmdCount++;
                    break;
                default:
                    break;
            }
            
        }
        u32Count++;
    }
    
    switch(u32ArgIdx)
    {
        case 1:
            memcpy(chArg,chArg1,sizeof(HI_CHAR)*DEF_FILE_NAMELENGTH);
            break;
        case 2:
            memcpy(chArg,chArg2,sizeof(HI_CHAR)*DEF_FILE_NAMELENGTH);
            break;
        default:
            break;
    }
    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype    : hdmi_Proc
 Description  : HDMI status in /proc/msp/hdmi0
 Input        : None
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/

static HI_S32 HDMI0_Proc(struct seq_file *p, HI_VOID *v)
{
    HI_U32 u32Reg, index, offset,u32DefHDMIMode;
	HDMI_ATTR_S			          stHDMIAttr; 
    HDMI_VIDEO_ATTR_S            *pstVideoAttr;
    HDMI_AUDIO_ATTR_S            *pstAudioAttr;
    HDMI_APP_ATTR_S              *pstAppAttr;
    HI_UNF_HDMI_STATUS_S          stHdmiStatus;
    HI_UNF_HDMI_CEC_STATUS_S      CECStatus;
    HI_U32                        u32PlayStatus = 0;
    HI_S32 s32Temp, Ret = HI_SUCCESS;;

    p += PROC_PRINT(p, "--------------------------------- Hisi HDMI Dev Stat --------------------------------\n");
    Ret = DRV_HDMI_GetAttr(HI_UNF_HDMI_ID_0, &stHDMIAttr);
    if(Ret != HI_SUCCESS)
    {
        p += PROC_PRINT(p, "HDMI driver do not Open\n" );
        p += PROC_PRINT(p, "----------------------------------------- END -----------------------------------------\n");
        return HI_SUCCESS;
    }
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV0, 0x08);// 0x72:0x08
    if ((u32Reg & 0x01) != 0x01)
    {
        p += PROC_PRINT(p, "HDMI do not Start!\n");
        p += PROC_PRINT(p, "----------------------------------------- END -----------------------------------------\n");
        return HI_SUCCESS;
    }
    pstVideoAttr = &stHDMIAttr.stVideoAttr;
    pstAudioAttr = &stHDMIAttr.stAudioAttr;
    pstAppAttr = &stHDMIAttr.stAppAttr;

    DRV_HDMI_GetStatus(HI_UNF_HDMI_ID_0,&stHdmiStatus);
    
    p += PROC_PRINT(p, "%-20s: ","Hotplug");
    if(stHdmiStatus.bConnected)
    {
        p += PROC_PRINT(p, "%-20s| ", "Enable");
    }
    else
    { 
        p += PROC_PRINT(p, "%-20s| ", "Disable");
    }
    p += PROC_PRINT(p, "%-20s: ","Thread");
    s32Temp = (DRV_Get_IsChnOpened(HI_UNF_HDMI_ID_0) && !DRV_Get_IsThreadStoped() && !SI_IsHDMIResetting()) ;
    if(s32Temp)
    {
        p += PROC_PRINT(p, "%s\n", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Disable");
    }
    
    p += PROC_PRINT(p, "%-20s: ","Sink");
    if(stHdmiStatus.bSinkPowerOn)
    {
        p += PROC_PRINT(p, "%-20s| ", "Active");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Deactive");
    }
    p += PROC_PRINT(p, "%-20s: ","HDCP Enable");
    if(pstAppAttr->bHDCPEnable)
    {
        p += PROC_PRINT(p, "%s\n", "ON");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "OFF");
    }

    p += PROC_PRINT(p, "%-20s: ","PHY Output");
    s32Temp = DRV_HDMI_ReadPhy();
    if(s32Temp)
    {
        p += PROC_PRINT(p, "%-20s| ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Disable");
    }
    p += PROC_PRINT(p, "%-20s: ","HDCP Encryption");
    s32Temp = DRV_ReadByte_8BA(0, TX_SLV0, 0x0F);
    if(s32Temp&0x01)
    {
        p += PROC_PRINT(p, "%s\n", "ON");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "OFF");
    }

    p += PROC_PRINT(p, "%-20s: ","Play Status");
    DRV_HDMI_GetPlayStatus(0,&u32PlayStatus);
    if(u32PlayStatus)
    {
        p += PROC_PRINT(p, "%-20s| ", "Start");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Stop");
    }
    p += PROC_PRINT(p, "%-20s: ","CEC Status");
    memset(&CECStatus, 0, sizeof(HI_UNF_HDMI_CEC_STATUS_S));
#ifdef CEC_SUPPORT
    DRV_HDMI_CECStatus(HI_UNF_HDMI_ID_0, &CECStatus);
#endif
    if(CECStatus.bEnable)
    {
        p += PROC_PRINT(p, "%s\n", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Disable");
    }

    p += PROC_PRINT(p, "%-20s: ","EDID Status");
    if(DRV_Get_IsValidSinkCap(HI_UNF_HDMI_ID_0))
    {
        p += PROC_PRINT(p, "%-20s| ", "Valid");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Unvalid");
    }
    p += PROC_PRINT(p, "%-20s: ","CEC Phy Addr");
   
    p += PROC_PRINT(p, "%02d.%02d.%02d.%02d\n", CECStatus.u8PhysicalAddr[0],
        CECStatus.u8PhysicalAddr[1],CECStatus.u8PhysicalAddr[2],CECStatus.u8PhysicalAddr[3]);


    p += PROC_PRINT(p, "%-20s: ","Default Mode");
    u32DefHDMIMode = DRV_Get_DefaultOutputMode(HI_UNF_HDMI_ID_0);
    p += PROC_PRINT(p, "%-20s| ", g_pDefHDMIMode[u32DefHDMIMode]);
    p += PROC_PRINT(p, "%-20s: ","CEC Logical Addr");
    p += PROC_PRINT(p, "%d\n", CECStatus.u8LogicalAddr);


    p += PROC_PRINT(p, "%-20s: ","Output Mode");
    u32Reg = ReadByteHDMITXP1(0x2F);
    if(u32Reg & 0x01)
    {
        p += PROC_PRINT(p, "%-20s| ", "HDMI");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "DVI");
    }

    p += PROC_PRINT(p, "%-20s: ","AVMUTE");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0xDF);  // 0x7A:0xDF
    if ( 0x10 == (u32Reg & 0x10))
    {
        p += PROC_PRINT(p, "%s ", "Disable");
    }
    else if ( 0x01 == (u32Reg & 0x01))
    {
        p += PROC_PRINT(p, "%s ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%s ", "Unknown");
    }    
    p += PROC_PRINT(p, "\n");

    
    p += PROC_PRINT(p, "%-20s: ","Force SetFmt Delay");
    if(IsForceFmtDelay())
    {
        p += PROC_PRINT(p, "%-20s| ", "Force");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Default");
    }

    p += PROC_PRINT(p, "%-20s: ","Force Mute Delay");
    if (IsForceMuteDelay())
    {
        p += PROC_PRINT(p, "%s ", "Force");
    }
    else
    {
        p += PROC_PRINT(p, "%s ", "Default");
    }
    p += PROC_PRINT(p, "\n");
    
    p += PROC_PRINT(p, "---------------- Video -------------------|---------------- Audio -------------------\n");
    
    p += PROC_PRINT(p, "%-20s: ","Video Output ");
    if(pstAppAttr->bEnableVideo)
    {
        p += PROC_PRINT(p, "%-20s| ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Disable");
    }
    p += PROC_PRINT(p, "%-20s: ","AUD Output");
    if(pstAppAttr->bEnableAudio)
    {
        p += PROC_PRINT(p, "%s\n", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Disable");
    }

    p += PROC_PRINT(p, "%-20s: ","Current Fmt");
    p += PROC_PRINT(p, "%-20s| ", g_pDispFmtString[pstVideoAttr->enVideoFmt]);
    p += PROC_PRINT(p, "%-20s: ","Input Type");
    p += PROC_PRINT(p, "%s\n", g_pAudInputType[pstAudioAttr->enSoundIntf]);

    p += PROC_PRINT(p, "%-20s: ","Color Space");
    p += PROC_PRINT(p, "%-20s| ", g_pColorSpace[pstAppAttr->enVidOutMode]);
    p += PROC_PRINT(p, "%-20s: ","Sample Rate");
    p += PROC_PRINT(p, "%dHZ\n", pstAudioAttr->enSampleRate);

    p += PROC_PRINT(p, "%-20s: ","DeepColor");
    p += PROC_PRINT(p, "%-20s| ", g_pDeepColor[pstAppAttr->enDeepColorMode]);
    p += PROC_PRINT(p, "%-20s: ","Bit Depth");
    p += PROC_PRINT(p, "%dbit\n", pstAudioAttr->enBitDepth);


    p += PROC_PRINT(p, "%-20s: ","xvYCC");
    if(pstAppAttr->bxvYCCMode)
    {
        p += PROC_PRINT(p, "%-20s| ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Disable");
    }
    p += PROC_PRINT(p, "%-20s: ","Trace Mode");
    if(pstAudioAttr->bIsMultiChannel)
    {
        p += PROC_PRINT(p, "%s\n", "Multichannel(8)");        
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Stereo");
    }
    
    p += PROC_PRINT(p, "%-20s: ","3D Mode");
    if(0 == pstVideoAttr->u83DParam)
    {
        p += PROC_PRINT(p, "%-20s| ", "FPK");
    }
    else if (8 == pstVideoAttr->u83DParam)
    {
        p += PROC_PRINT(p, "%-20s| ", "SBS HALF");
    }
    else if (6 == pstVideoAttr->u83DParam)
    {
        p += PROC_PRINT(p, "%-20s| ", "TAB");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "2D");
    }
    
    p += PROC_PRINT(p, "%-20s: ","N Value");
    u32Reg = ReadByteHDMITXP1(0x05);  // 0x7A:0x05
    u32Reg = (u32Reg<<8) | ReadByteHDMITXP1(0x04);  // 0x7A:0x04
    u32Reg = (u32Reg<<8) | ReadByteHDMITXP1(0x03);  // 0x7A:0x03
    p += PROC_PRINT(p, "0x%x(%d)\n",u32Reg,u32Reg);
    
    p += PROC_PRINT(p, "%-20s: ","Global SetFmt Delay");
    p += PROC_PRINT(p, "%-20d| ", GetGlobalFmtDelay());


    p += PROC_PRINT(p, "%-20s: ","CTS");
    u32Reg = ReadByteHDMITXP1(0x0b);  // 0x7A:0x0b
    u32Reg = (u32Reg<<8) | ReadByteHDMITXP1(0x0a);  // 0x7A:0x0a
    u32Reg = (u32Reg<<8) | ReadByteHDMITXP1(0x09);  // 0x7A:0x09
    p += PROC_PRINT(p, "0x%x(%d)\n",u32Reg,u32Reg);

    p += PROC_PRINT(p, "%-20s: ","Global Mute Delay");
    p += PROC_PRINT(p, "%-20d| ",GetGlobalsMuteDelay());
    p += PROC_PRINT(p, "\n");
    
    p += PROC_PRINT(p, "---------------------------------- Info Frame status --------------------------------\n");
    p += PROC_PRINT(p, "%-25s: ","AVI InfoFrame");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x3E);  // 0x7A:0x3E
    if ( 0x03 == (u32Reg & 0x03))
    {
        p += PROC_PRINT(p, "%-15s| ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%-15s| ", "Disable");
    }
    p += PROC_PRINT(p, "%-23s: ","Gamut Metadata Packet");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x3F);  // 0x7A:0x3F
    if(0xC0 == (u32Reg & 0xC0))
    {
        p += PROC_PRINT(p, "%s\n", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Disable");
    }

    p += PROC_PRINT(p, "%-25s: ","AUD InfoFrame");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x3E);  // 0x7A:0x3E
    if ( 0x30 == (u32Reg & 0x30))
    {
        p += PROC_PRINT(p, "%-15s| ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%-15s| ", "Disable");
    }
    p += PROC_PRINT(p, "%-23s: ","Generic Packet");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x3F);  // 0x7A:0x3F
    if(0x03 == (u32Reg & 0x03))
    {
        p += PROC_PRINT(p, "%s\n", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Disable");
    }

    p += PROC_PRINT(p, "%-25s: ","MPg/VendorSpec InfoFrame");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x3E);  // 0x7A:0x3E
    if ( 0xC0 == (u32Reg & 0xC0))
    {
        p += PROC_PRINT(p, "%-15s| ", "Enable");
    }
    else
    {
        p += PROC_PRINT(p, "%-15s| ", "Disable");
    }
    p += PROC_PRINT(p, "\n");
    p += PROC_PRINT(p, "-------------------------------------- Raw Data -------------------------------------\n");
    p += PROC_PRINT(p, "AVI InfoFrame :\n");
    for(index = 0; index < 17; index ++)
    {
        offset = 0x40 + index;//0x7A:0x40
        u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, offset);
        p += PROC_PRINT(p, "0x%02x,", u32Reg);
    }
    p += PROC_PRINT(p, "\n");
    p += PROC_PRINT(p, "AUD InfoFrame :\n");
    for(index = 0; index < 9; index ++)
    {
        offset = 0x80 + index;//0x7A:0x80
        u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, offset);
        p += PROC_PRINT(p, "0x%02x,", u32Reg);
    }
    p += PROC_PRINT(p, "\n");
    p += PROC_PRINT(p, "MPg/VendorSpec Inforframe :\n");
    for(index = 0; index < 12; index ++)
    {
        offset = 0xa0 + index;//0x7A:0xA0
        u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, offset);
        p += PROC_PRINT(p, "0x%02x,", u32Reg);
    }
    p += PROC_PRINT(p, "\n");
    p += PROC_PRINT(p, "------------------------------------ Parsed InfoFrame -------------------------------\n");
    p += PROC_PRINT(p, "%-20s: ","Video ID Code(VIC)");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x47);
    p += PROC_PRINT(p, "0x%-18x| ", u32Reg);
    p += PROC_PRINT(p, "%-20s: ","Colorimetry");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x45);
    u32Reg = u32Reg>>6;
    if(!u32Reg)
    {
        p += PROC_PRINT(p, "%s\n", "No Data");
    }
    else if (0x1 == u32Reg)
    {
        p += PROC_PRINT(p, "%s\n", "ITU601");
    }
    else if (0x2 == u32Reg)
    {
        p += PROC_PRINT(p, "%s\n", "ITU709");
    }
    else
    {
        u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x46);
        u32Reg &= 0x70;
        if(u32Reg == 0)
        {
            p += PROC_PRINT(p, "%s\n", "xvYCC601");
        }
        else if(u32Reg == 0x10)
        {
            p += PROC_PRINT(p, "%s\n", "xvYCC709");
        }
        else
        {
            p += PROC_PRINT(p, "%s\n", "reserve");
        }
    }
    p += PROC_PRINT(p, "%-20s: ","Pixel Repetition");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x48);
    u32Reg &= 0x0F;
    if(u32Reg > 10)
    {
        //array overflow, set to reserved
        u32Reg = 10;
    }
    p += PROC_PRINT(p, "%-20s| ", g_pPixelRep[u32Reg]);
    p += PROC_PRINT(p, "%-20s: ","ScanInfo");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x44);
    u32Reg &= 0x03;
    p += PROC_PRINT(p, "%s\n", g_pScanInfo[u32Reg]);

    p += PROC_PRINT(p, "%-20s: ","Output Color Space");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x44);
    u32Reg &= 0x60;
    u32Reg >>=5;
    p += PROC_PRINT(p, "%-20s| ", g_pColorSpace[u32Reg]);
    p += PROC_PRINT(p, "%-20s: ","AspectRatio");
    u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x45);
    u32Reg &= 0x30;
    if(0x00 == u32Reg)
    {
        p += PROC_PRINT(p, "%s\n", "No Data");
    }
    else if (0x10 == u32Reg)
    {
        p += PROC_PRINT(p, "%s\n", "4:3");
    }
    else if (0x20 == u32Reg)
    {
        p += PROC_PRINT(p, "%s\n", "16:9");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "Future");
    }

    p += PROC_PRINT(p, "--------------------------------- Debug Command -------------------------------------\n");
    p += PROC_PRINT(p, "type 'echo help > /proc/msp/hdmi0' to get help informatin \n");
    p += PROC_PRINT(p, "---------------------------------------- END ----------------------------------------\n");
  
    return HI_SUCCESS;
}

static HI_S32 HDMI0_Sink_Proc(struct seq_file *p, HI_VOID *v)
{
    HI_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(HI_UNF_HDMI_ID_0);
    HI_DRV_HDMI_AUDIO_CAPABILITY_S *pOldAudioCap = DRV_Get_OldAudioCap();    
    HI_U32 i,j;    

    p += PROC_PRINT(p, "--------------------------------- Hisi HDMI Sink Capability -------------------------\n");
    
    p += PROC_PRINT(p, "%-20s: ","EDID Status");
    if(DRV_Get_IsValidSinkCap(HI_UNF_HDMI_ID_0))
    {
        p += PROC_PRINT(p, "%-20s| ", "OK");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "Failed");
    }
    p += PROC_PRINT(p, "%-20s: ","TV Manufacture Name");
    p += PROC_PRINT(p, "%s\n", pSinkCap->stMfrsInfo.u8MfrsName);

    p += PROC_PRINT(p, "%-20s: ","Source of EDID");

    if(DRV_Get_IsUserEdid(HI_UNF_HDMI_ID_0))
    {
        p += PROC_PRINT(p, "%-20s| ","User Setting");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ","From Sink");
    }
    
    p += PROC_PRINT(p, "%-20s: ","ProductCode");
    p += PROC_PRINT(p, "%d\n", pSinkCap->stMfrsInfo.u32ProductCode);

    p += PROC_PRINT(p, "%-20s: ","Hdmi Support");
    if(pSinkCap->bSupportHdmi)
    {
        p += PROC_PRINT(p, "%-20s| ", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "FALSE");
    }
    p += PROC_PRINT(p, "%-20s: ","SerialNumber");
    p += PROC_PRINT(p, "%d\n", pSinkCap->stMfrsInfo.u32SerialNumber);

    p += PROC_PRINT(p, "%-20s: ","EDID Version");
    p += PROC_PRINT(p, "%d.%-18d| ", pSinkCap->u8Version,pSinkCap->u8Revision);
    p += PROC_PRINT(p, "%-20s: ","Week Of Manufacture");
    p += PROC_PRINT(p, "%d\n", pSinkCap->stMfrsInfo.u32Week);

    p += PROC_PRINT(p, "%-20s: ","Extend Block Num");
    p += PROC_PRINT(p, "%-20d| ", pSinkCap->u8ExtBlockNum);
    p += PROC_PRINT(p, "%-20s: ","Year Of Manufacture");
    p += PROC_PRINT(p, "%d\n", pSinkCap->stMfrsInfo.u32Year);

    p += PROC_PRINT(p, "%-20s: ","DVI Dual");
    if(pSinkCap->bSupportDVIDual)
    {
        p += PROC_PRINT(p, "%-20s| ", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "FALSE");
    }
    p += PROC_PRINT(p, "%-20s: ","CEC PhyAddr Valid");
    if(pSinkCap->stCECAddr.bPhyAddrValid)
    {
        p += PROC_PRINT(p, "%s\n", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "FALSE");
    }

    p += PROC_PRINT(p, "%-20s: ","Supports AI");
    if(pSinkCap->bSupportsAI)
    {
        p += PROC_PRINT(p, "%-20s| ", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "FALSE");
    }
    p += PROC_PRINT(p, "%-20s: ","CEC Phy Add");
    p += PROC_PRINT(p, "%02x.%02x.%02x.%02x\n", pSinkCap->stCECAddr.u8PhyAddrA, 
        pSinkCap->stCECAddr.u8PhyAddrB, pSinkCap->stCECAddr.u8PhyAddrC, pSinkCap->stCECAddr.u8PhyAddrD);
    p += PROC_PRINT(p, "-------------------------------------- Video ----------------------------------------\n");
    p += PROC_PRINT(p, "%-20s: ","Video Timing");
    for(i = 0,j = 0; i < HI_UNF_ENC_FMT_BUTT; i++)
    {
        if(pSinkCap->bSupportFormat[i])
        {
            p += PROC_PRINT(p, "%s / ", g_pUnfFmtString[i]);
            j++;
            if(0 == j%6)
            {
                p += PROC_PRINT(p, "\n%-22s","");
            }
        }
    }
    p += PROC_PRINT(p, "\n");
    p += PROC_PRINT(p, "%-20s: ","Native Format");
    p += PROC_PRINT(p, "%s\n", g_pUnfFmtString[pSinkCap->enNativeFormat]);
    p += PROC_PRINT(p, "%-20s: ","Colorimetry");
    if(pSinkCap->stColorMetry.bxvYCC601)
    {
        p += PROC_PRINT(p, " / %s", "xvYCC601");
    }
    if(pSinkCap->stColorMetry.bxvYCC709)
    {
        p += PROC_PRINT(p, " / %s", "xvYCC709");
    }
    p += PROC_PRINT(p, "\n");
    
    p += PROC_PRINT(p, "%-20s: ","Color Space");
    p += PROC_PRINT(p, "%s", "RGB444");
    if(pSinkCap->stColorSpace.bYCbCr444)
    {
        p += PROC_PRINT(p, " / %s", "YCbCr444");
    }

    if(pSinkCap->stColorSpace.bYCbCr422)
    {
        p += PROC_PRINT(p, " / %s", "YCbCr422");
    }
    
    p += PROC_PRINT(p, "\n");

    p += PROC_PRINT(p, "%-20s: ","Deep Color");
    p += PROC_PRINT(p, "%s", "24");
    if(pSinkCap->stDeepColor.bDeepColor30Bit)
    {
        p += PROC_PRINT(p, " / %s", "30");
    }
    if(pSinkCap->stDeepColor.bDeepColor36Bit)
    {
        p += PROC_PRINT(p, " / %s", "36");
    }
    if(pSinkCap->stDeepColor.bDeepColor48Bit)
    {
        p += PROC_PRINT(p, " / %s", "48");
    }
    p += PROC_PRINT(p, "bit\n");
    
    p += PROC_PRINT(p, "%-20s: ","3D Support");
    if(pSinkCap->st3DInfo.bSupport3D)
    {
        p += PROC_PRINT(p, "%s\n","Support");
        p += PROC_PRINT(p, "%-20s: ","3D Type");
        for(i = 0;i < HI_UNF_EDID_3D_BUTT;i++)
        {
            if(pSinkCap->st3DInfo.bSupport3DType[i])
            {                  
                p += PROC_PRINT(p, "%-20s: ",g_p3DMode[i]);
            }
        }
    }
    else
    {
        p += PROC_PRINT(p, "%s\n","Not Support");
        p += PROC_PRINT(p, "%-20s: None","3D Type");
    }

    p += PROC_PRINT(p, "\n");    

    p += PROC_PRINT(p, "-------------------------------------- Audio ----------------------------------------\n");
    
    p += PROC_PRINT(p, "%-16s| ","Audio Fmt");
    p += PROC_PRINT(p, "%-15s| ","Chn");
    p += PROC_PRINT(p, "%-25s","samplerate");
    //p += PROC_PRINT(p, "%-10s","Extend");
    p += PROC_PRINT(p, "\n");

    for(i = 0; i < HI_UNF_EDID_MAX_AUDIO_CAP_COUNT; i++)
    {
        if(pSinkCap->stAudioInfo[i].enAudFmtCode)
        {
            p += PROC_PRINT(p, "%-3d %-12s| ",pSinkCap->stAudioInfo[i].enAudFmtCode, g_pAudioFmtCode[pSinkCap->stAudioInfo[i].enAudFmtCode]);
            p += PROC_PRINT(p, "%-15d| ",pSinkCap->stAudioInfo[i].u8AudChannel);

            for (j = 0; j < MAX_SAMPE_RATE_NUM; j++)
            {
                if(pSinkCap->stAudioInfo[i].enSupportSampleRate[j] != 0)
                {
                    p += PROC_PRINT(p, "%d ", (pSinkCap->stAudioInfo[i].enSupportSampleRate[j]));
                }
            }
            p += PROC_PRINT(p, "Hz");

            p += PROC_PRINT(p, "\n");
        }
    }

    p += PROC_PRINT(p, "\n%-10s : %d \n","Audio Info Num",pSinkCap->u32AudioInfoNum);        
    p += PROC_PRINT(p, "\n%-10s : ","Speaker");
        

    for(i = 0; i < HI_UNF_EDID_AUDIO_SPEAKER_BUTT; i++)
    {
        if(pSinkCap->bSupportAudioSpeaker[i])
        {
            p += PROC_PRINT(p, "%s ",g_pSpeaker[i]);
        }        
    }
            
    p += PROC_PRINT(p, "\n");

    p += PROC_PRINT(p, "------------------------------------ Old Audio --------------------------------------\n");
    p += PROC_PRINT(p, "Audio Fmt support : ");    
    for (i = 0; i < HI_UNF_EDID_AUDIO_FORMAT_CODE_BUTT; i++)
    {
        if (pOldAudioCap->bAudioFmtSupported[i] == HI_TRUE)
        {
             switch (i)
            {
            case 1:
                p += PROC_PRINT(p, "LiniarPCM ");
                break;
            case 2:
                p += PROC_PRINT(p, "AC3 ");
                break;
            case 3:
                p += PROC_PRINT(p, "MPEG1 ");
                break;
            case 4:
                p += PROC_PRINT(p, "MP3 ");
                break;
            case 5:
                p += PROC_PRINT(p, "MPEG2 ");
                break;
            case 6:
                p += PROC_PRINT(p, "ACC ");
                break;
            case 7:
                p += PROC_PRINT(p, "DTS ");
                break;
            case 8:
                p += PROC_PRINT(p, "ATRAC ");
                break;
            case 9:
                p += PROC_PRINT(p, "OneBitAudio ");
                break;
            case 10:
                p += PROC_PRINT(p, "DD+ ");
                break;
            case 11:
                p += PROC_PRINT(p, "DTS_HD ");
                break;
            case 12:
                p += PROC_PRINT(p, "MAT ");
                break;
            case 13:
                p += PROC_PRINT(p, "DST ");
                break;
            case 14:
                p += PROC_PRINT(p, "WMA ");
                break;
            default:
                p += PROC_PRINT(p, "reserved "); 
                break;
            }
        }
    }
    p += PROC_PRINT(p, "\n");

    p += PROC_PRINT(p, "Max Audio PCM channels: %d\n", pOldAudioCap->u32MaxPcmChannels);
    p += PROC_PRINT(p, "Support Audio Sample Rates:");
    for (i = 0; i < MAX_SAMPE_RATE_NUM; i++)
    {
        if(pOldAudioCap->u32AudioSampleRateSupported[i] != 0)
        {
            p += PROC_PRINT(p, " %d ", pOldAudioCap->u32AudioSampleRateSupported[i]);
        }
    }
    p += PROC_PRINT(p, "\n");

    p += PROC_PRINT(p, "------------------------------ Custom Perfer Timing ---------------------------------\n");

    p += PROC_PRINT(p, "%-20s: ","VFB");
    p += PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VFB);    

    p += PROC_PRINT(p, "%-20s: ","HFB");
    p += PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HFB);

    p += PROC_PRINT(p, "%-20s: ","VBB");
    p += PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VBB);    

    p += PROC_PRINT(p, "%-20s: ","HBB");
    p += PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HBB);
    
    p += PROC_PRINT(p, "%-20s: ","VACT");
    p += PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VACT);    

    p += PROC_PRINT(p, "%-20s: ","HACT");
    p += PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HACT);

    p += PROC_PRINT(p, "%-20s: ","VPW");
    p += PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VPW);    

    p += PROC_PRINT(p, "%-20s: ","HPW");
    p += PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HPW);


    p += PROC_PRINT(p, "%-20s: ","IDV");
    if(pSinkCap->stPerferTiming.bIDV)
    {
        p += PROC_PRINT(p, "%-20s| ", "TRUE");    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "FALSE");
    }

    p += PROC_PRINT(p, "%-20s: ","IHS");
    if(pSinkCap->stPerferTiming.bIHS)
    {
        p += PROC_PRINT(p, "%s\n", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%s\n", "FALSE");
    }

    
    p += PROC_PRINT(p, "%-20s: ","IVS");
    if(pSinkCap->stPerferTiming.bIVS)
    {
        p += PROC_PRINT(p, "%-20s| \n", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| \n", "FALSE");
    }


    p += PROC_PRINT(p, "%-20s: ","Image Width");
    p += PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32ImageWidth);    

    p += PROC_PRINT(p, "%-20s: ","Image Height");
    p += PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32ImageHeight);

    
    p += PROC_PRINT(p, "%-20s: ","Interlace");

    if(pSinkCap->stPerferTiming.bInterlace)
    {
        p += PROC_PRINT(p, "%-20s| ", "TRUE");
    }
    else
    {
        p += PROC_PRINT(p, "%-20s| ", "FALSE");
    }  

    p += PROC_PRINT(p, "%-20s: ","Pixel Clock");
    p += PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32PixelClk);

    p += PROC_PRINT(p, "------------------------------------ EDID Raw Data ---------------------------------- \n");

    if(!DRV_Get_IsValidSinkCap(HI_UNF_HDMI_ID_0))
    {
        p += PROC_PRINT(p, "!! Data unbelievably !! \n");
    }
    //no else
    {
        HI_U32 index,u32EdidLegth = 0;
        HI_U8  Data[512];

        memset(Data, 0, 512);
        u32EdidLegth = 128*(pSinkCap->u8ExtBlockNum + 1);

        if(u32EdidLegth > 512)
        {
            u32EdidLegth = 512;
        }
        
        SI_Proc_ReadEDIDBlock(Data, u32EdidLegth);
        for (index = 0; index < u32EdidLegth; index ++)
        {
            p += PROC_PRINT(p, "%02x ", Data[index]);
            if (0 == ((index + 1) % 16))
            {
                p += PROC_PRINT(p, "\n");
            }
        }
    }
        
    p += PROC_PRINT(p, "---------------------------------------- END ----------------------------------------\n");
       
    return HI_SUCCESS;
}

extern HI_U32 unStableTimes;

HI_S32 hdmi_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
#ifndef HI_ADVCA_FUNCTION_RELEASE
    //struct seq_file   *p = file->private_data;
    //DRV_PROC_ITEM_S  *pProcItem = s->private;
    HI_CHAR  chCmd[60] = {0};
    HI_CHAR  chArg1[DEF_FILE_NAMELENGTH] = {0};
    HI_CHAR  chArg2[DEF_FILE_NAMELENGTH] = {0};

    
    if(count > 40)
    {   
        HI_DRV_PROC_EchoHelper("Error:Echo too long.\n");
        return HI_FAILURE;
    }
    
    if(copy_from_user(chCmd,buf,count))
    {
        HI_DRV_PROC_EchoHelper("copy from user failed\n");
        return HI_FAILURE;
    }

    hdmi_GetProcArg(chCmd, chArg1, 1);
    hdmi_GetProcArg(chCmd, chArg2, 2);

    //sw reset
    if(!HI_OSAL_Strncmp(chArg1,"swrst",DEF_FILE_NAMELENGTH))
    {
        HI_DRV_PROC_EchoHelper("hdmi resetting... ... ... \n");
        SI_SW_ResetHDMITX();
    }
    //invert tmds clock
    else if(!HI_OSAL_Strncmp(chArg1,"tclk",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("hdmi TmdsClk invert...  \n");
            WriteByteHDMITXP1(0x3d,0x1f);
        }
        else if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("hdmi TmdsClk not invert... \n");
            WriteByteHDMITXP1(0x3d,0x17);
        }
    }
    else if(!HI_OSAL_Strncmp(chArg1,"mute",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("mute...  \n");
            DRV_HDMI_SetAVMute(0,HI_TRUE);
            //WriteByteHDMITXP1(0x3d,0x1f);
        }
        else if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("unmute... \n");
            DRV_HDMI_SetAVMute(0,HI_FALSE);
            //WriteByteHDMITXP1(0x3d,0x17);
        }
    }
    //else if(chArg1[0] == '3'&& chArg1[1] == 'd')
    else if(!HI_OSAL_Strncmp(chArg1,"3d",DEF_FILE_NAMELENGTH))
    {
        HDMI_ATTR_S stAttr;
        DRV_HDMI_GetAttr(0,&stAttr);
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("3d mode disable...  \n");
            //HI_DRV_HDMI_Set3DMode(0,HI_FALSE,HI_UNF_3D_MAX_BUTT);
            stAttr.stVideoAttr.b3DEnable = HI_FALSE;
            stAttr.stVideoAttr.u83DParam = HI_UNF_EDID_3D_BUTT;
            //WriteByteHDMITXP1(0x3d,0x1f);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"fp",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("Frame Packing... \n");
            //HI_DRV_HDMI_Set3DMode(0,HI_TRUE,HI_UNF_3D_FRAME_PACKETING);
            stAttr.stVideoAttr.b3DEnable = HI_TRUE;
            stAttr.stVideoAttr.u83DParam = HI_UNF_EDID_3D_FRAME_PACKETING;
            //WriteByteHDMITXP1(0x3d,0x17);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"sbs",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("Side by side(half)... \n");
            //HI_DRV_HDMI_Set3DMode(0,HI_TRUE,HI_UNF_3D_SIDE_BY_SIDE_HALF);
            stAttr.stVideoAttr.b3DEnable = HI_TRUE;
            stAttr.stVideoAttr.u83DParam = HI_UNF_EDID_3D_SIDE_BY_SIDE_HALF;
            //WriteByteHDMITXP1(0x3d,0x17);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"tab",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("Top and bottom... \n");
            //HI_DRV_HDMI_Set3DMode(0,HI_TRUE,HI_UNF_3D_TOP_AND_BOTTOM);
            stAttr.stVideoAttr.b3DEnable = HI_TRUE;
            stAttr.stVideoAttr.u83DParam = HI_UNF_EDID_3D_TOP_AND_BOTTOM;
            //WriteByteHDMITXP1(0x3d,0x17);
        }
        DRV_HDMI_SetAttr(0,&stAttr);
    }
    else if(!HI_OSAL_Strncmp(chArg1,"cbar",DEF_FILE_NAMELENGTH))
    {
        HI_U32 u32Reg = 0;
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("colorbar disable...  \n");
            DRV_HDMI_ReadRegister(0xf8ccc000,&u32Reg);
            u32Reg = u32Reg & (~0x70000000);
            DRV_HDMI_WriteRegister(0xf8ccc000,(u32Reg | 0x1));
        }
        else if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("colorbar enable.. \n");
            DRV_HDMI_ReadRegister(0xf8ccc000,&u32Reg);
            u32Reg = u32Reg | 0x70000000;
            DRV_HDMI_WriteRegister(0xf8ccc000,(u32Reg | 0x1));
        }
    }
    // 0x00 0xff 0xff yellow
    // 0x00 0xff 0x00 green
    // 0xff 0x00 0x00 blue
    // 0x00 0x00 0xff red
    // 0x80 0x10 0x80 black
    // white
    else if(!HI_OSAL_Strncmp(chArg1,"vblank",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("vblank disable...  \n");
            WriteByteHDMITXP0(0x0d,0x00);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"black",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("vblank black.. \n");
            WriteByteHDMITXP0(0x0d,0x04);
            WriteByteHDMITXP0(0x4b,0x80);
            WriteByteHDMITXP0(0x4c,0x10);
            WriteByteHDMITXP0(0x4d,0x80);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"red",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("vblank red.. \n");
            WriteByteHDMITXP0(0x0d,0x04);
            WriteByteHDMITXP0(0x4b,0x00);
            WriteByteHDMITXP0(0x4c,0x00);
            WriteByteHDMITXP0(0x4d,0xff);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"green",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("vblank green.. \n");
            WriteByteHDMITXP0(0x0d,0x04);
            WriteByteHDMITXP0(0x4b,0x00);
            WriteByteHDMITXP0(0x4c,0xff);
            WriteByteHDMITXP0(0x4d,0x00);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"blue",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("vblank blue.. \n");
            WriteByteHDMITXP0(0x0d,0x04);
            WriteByteHDMITXP0(0x4b,0xff);
            WriteByteHDMITXP0(0x4c,0x00);
            WriteByteHDMITXP0(0x4d,0x00);
        }
    }
    else if(!HI_OSAL_Strncmp(chArg1,"enc",DEF_FILE_NAMELENGTH))
    {
        if(!HI_OSAL_Strncmp(chArg2,"phy",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("encode by phy \n");
            WriteByteHDMITXP1(0x3c,0x08);
            SI_TX_PHY_WriteRegister(0x0d,0x00);
        }
        else if(!HI_OSAL_Strncmp(chArg2,"ctrl",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("encode by ctrl \n");
            WriteByteHDMITXP1(0x3c,0x00);
            SI_TX_PHY_WriteRegister(0x0d,0x01);
        }
    }
    else if(!HI_OSAL_Strncmp(chArg1,"pclk",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("pclk nobypass \n");
            WriteByteHDMITXP1(0x3c,0x08);
        }
        else if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("pclk bypass \n");
            WriteByteHDMITXP1(0x3c,0x28);
        }
    }    
    else if(!HI_OSAL_Strncmp(chArg1,"audio",DEF_FILE_NAMELENGTH))
    {
        HDMI_AUDIO_ATTR_S stHDMIAOAttr;
        memset((void*)&stHDMIAOAttr, 0, sizeof(HDMI_AUDIO_ATTR_S));
        DRV_HDMI_GetAOAttr(0,&stHDMIAOAttr);
        
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("audio I2S \n");
            stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
            DRV_HDMI_AudioChange(0,&stHDMIAOAttr);
        }
        else if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("audio SPDIF \n");
            stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
            DRV_HDMI_AudioChange(0,&stHDMIAOAttr);
        }
        else if(chArg2[0] == '2')
        {
            HI_DRV_PROC_EchoHelper("audio HBR \n");
            stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_HBR;
            DRV_HDMI_AudioChange(0,&stHDMIAOAttr);
        }
        else if(chArg2[0] == '3')
        {
            HI_DRV_PROC_EchoHelper("audio pcm 8ch 192Khz \n");
            stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
            stHDMIAOAttr.bIsMultiChannel = HI_TRUE;
            stHDMIAOAttr.u32Channels = 8;
            stHDMIAOAttr.enSampleRate = HI_UNF_SAMPLE_RATE_192K;
            DRV_HDMI_AudioChange(0,&stHDMIAOAttr);
        }
        else
        {
            HI_DRV_PROC_EchoHelper("not supported \n");
        }
    } 
    else if(!HI_OSAL_Strncmp(chArg1,"thread",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("thread stop \n");
            DRV_Set_ThreadStop(HI_TRUE);
        }
        else if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("thread start \n");          
            DRV_Set_ThreadStop(HI_FALSE);
        }
    }
    else if(!HI_OSAL_Strncmp(chArg1,"cec",DEF_FILE_NAMELENGTH))
    {
#if defined (CEC_SUPPORT)
        if(chArg2[0] == '0')
        {
            HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
            //cec_enable_flag = 0;
            SI_CEC_Close();
            DRV_Set_CECEnable(HI_UNF_HDMI_ID_0,HI_FALSE);
            DRV_Set_CECStart(HI_UNF_HDMI_ID_0,HI_FALSE);
            pstChnAttr[HI_UNF_HDMI_ID_0].u8CECCheckCount = 0;
            memset(&(pstChnAttr[HI_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(HI_UNF_HDMI_CEC_STATUS_S));
        }
        else if(chArg2[0] == '1')
        {
			SI_CEC_SetUp();
            DRV_Set_CECEnable(HI_UNF_HDMI_ID_0,HI_TRUE);
        }
#else
        HI_DRV_PROC_EchoHelper("do not support cec \n");
#endif
    }
    else if (!HI_OSAL_Strncmp(chArg1,"setAttr",DEF_FILE_NAMELENGTH))
    {
        HDMI_ATTR_S *pstHDMIAttr = DRV_Get_HDMIAttr(HI_UNF_HDMI_ID_0);
        DRV_Set_ForceUpdateFlag(HI_UNF_HDMI_ID_0,HI_TRUE);
        DRV_HDMI_SetAttr(HI_UNF_HDMI_ID_0, pstHDMIAttr);
    }
    else if (!HI_OSAL_Strncmp(chArg1,"check",DEF_FILE_NAMELENGTH))
    {
        HI_U32 u32Reg = 0;
        if(!HI_OSAL_Strncmp(chArg2,"timing",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("Check timing... \n");
            u32Reg = ReadByteHDMITXP0(TX_STAT_ADDR);

            if((u32Reg & BIT_HDMI_PSTABLE)!=0)
            {
                HI_DRV_PROC_EchoHelper("Pixel Clk      : stable \n");            
            }
            else
            {
                HI_DRV_PROC_EchoHelper("Pixel Clk      : !!Warnning!! Clock Unstable\n");
            }     
            
            HI_DRV_PROC_EchoHelper("Unstable Times : %d \n",unStableTimes);

            u32Reg = ReadByteHDMITXP0(0x3b);
            u32Reg = (u32Reg << 8) | ReadByteHDMITXP0(0x3a);        
            HI_DRV_PROC_EchoHelper("H Total        : %d ( 0x%x )\n",u32Reg,u32Reg);

            u32Reg = ReadByteHDMITXP0(0x3d);
            u32Reg = (u32Reg << 8) | ReadByteHDMITXP0(0x3c);
            
            HI_DRV_PROC_EchoHelper("V Total        : %d ( 0x%x )\n",u32Reg,u32Reg);

            u32Reg = ReadByteHDMITXP0(INTERLACE_POL_DETECT);
            if((u32Reg & BIT_I_DETECTR)!=0)
            {
                HI_DRV_PROC_EchoHelper("InterlaceDetect: interlace\n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("InterlaceDetect: progress\n");
            }

            u32Reg = ReadByteHDMITXP1(DIAG_PD_ADDR);

            HI_DRV_PROC_EchoHelper("Power State    : 0x%02x\n",u32Reg);


        }
        else if(!HI_OSAL_Strncmp(chArg2,"ddc",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("check ddc... \n");

            u32Reg = ReadByteHDMITXP0(DDC_DELAY_CNT);
            HI_DRV_PROC_EchoHelper("DDC Delay Count : 0x%02x(%d)\n",u32Reg,u32Reg);

            if(u32Reg > 0)
            {
                //60Mhz osc clk
                HI_DRV_PROC_EchoHelper("DDC Speed in calc : %dHz\n",(700000000 / (u32Reg * 35)));
            }
            else
            {
                //60Mhz osc clk
                HI_DRV_PROC_EchoHelper("DDC Speed in calc : 0Hz\n");
            }

            u32Reg = ReadByteHDMITXP0(DDC_STATUS);

            if(u32Reg & BIT_MDDC_ST_I2C_LOW)
            {
                HI_DRV_PROC_EchoHelper("I2C transaction did not start because I2C bus is pulled LOW by an external device \n");
            }

            if(u32Reg & BIT_MDDC_ST_NO_ACK)
            {
                HI_DRV_PROC_EchoHelper("HDMI Transmitter did not receive an ACK from slave device during address or data write \n");
            }
            if(u32Reg & BIT_MDDC_ST_IN_PROGR)
            {
                HI_DRV_PROC_EchoHelper("DDC operation in progress \n");
            }

            if(u32Reg & BIT_MDDC_ST_FIFO_FULL)
            {
                HI_DRV_PROC_EchoHelper("DDC FIFO Full \n");
            }

            if(u32Reg & BIT_MDDC_ST_FIFO_EMPL)
            {
                HI_DRV_PROC_EchoHelper("DDC FIFO Empty \n");
            }

            if(u32Reg & BIT_MDDC_ST_FRD_USE)
            {
                HI_DRV_PROC_EchoHelper("DDC FIFO Read In Use \n");
            }

            if(u32Reg & BIT_MDDC_ST_FWT_USE)
            {
                HI_DRV_PROC_EchoHelper("DDC FIFO Write In Use \n");
            }
        }
        else if(!HI_OSAL_Strncmp(chArg2,"color",DEF_FILE_NAMELENGTH))
        {
            //HI_DRV_PROC_EchoHelper("not supported... \n");
            HDMI_APP_ATTR_S  *pstAppAttr = DRV_Get_AppAttr(HI_UNF_HDMI_ID_0);

            if(pstAppAttr->bEnableHdmi)
            {
                HI_DRV_PROC_EchoHelper("ATTR HDMI Mode : HDMI \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("ATTR HDMI Mode : DVI \n");
            }

            if(pstAppAttr->bEnableAviInfoFrame)
            {
                HI_DRV_PROC_EchoHelper("ATTR AVI InfoFrame : Enable \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("ATTR AVI InfoFrame : Disable \n");
            }

            if(pstAppAttr->enVidOutMode == HI_UNF_HDMI_VIDEO_MODE_RGB444)
            {
                HI_DRV_PROC_EchoHelper("VIDEO Color Mode : RGB444 \n");
            }
            else if(pstAppAttr->enVidOutMode == HI_UNF_HDMI_VIDEO_MODE_YCBCR444)
            {
                HI_DRV_PROC_EchoHelper("VIDEO Color Mode : YCbCr444 \n");
            }
            else if(pstAppAttr->enVidOutMode == HI_UNF_HDMI_VIDEO_MODE_YCBCR422)
            {
                HI_DRV_PROC_EchoHelper("VIDEO Color Mode : YCBCR422(unsupport) \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("VIDEO Color Mode : Unknown Mode(ERR) \n");
            }
            
            HI_DRV_PROC_EchoHelper("InfoFrame Color Space:");
            u32Reg = DRV_ReadByte_8BA(0, TX_SLV1, 0x44);
            u32Reg &= 0x60;
            u32Reg >>=5;
            HI_DRV_PROC_EchoHelper("%s \n", g_pColorSpace[u32Reg]);

            u32Reg = ReadByteHDMITXP0(TX_VID_CTRL_ADDR);

            if(u32Reg & BIT_VID_CTRL_CSCSEL)
            {
                HI_DRV_PROC_EchoHelper("csc Standard select : BT709 \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("csc Standard select : BT601 \n");
            }

            
            u32Reg = ReadByteHDMITXP0(VID_ACEN_ADDR);
            
            if(u32Reg & BIT_VID_ACEN_RGB2YCBCR)
            {
                HI_DRV_PROC_EchoHelper("csc Rgb=>Yuv : Open \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("csc Rgb=>Yuv : close \n");
            }
            
            u32Reg = ReadByteHDMITXP0(TX_VID_MODE_ADDR);
            
            if(u32Reg & BIT_TX_CSC)
            {
                HI_DRV_PROC_EchoHelper("csc Yuv=>Rgb : Open \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("csc Yuv=>Rgb : close \n");
            }
            
            if(u32Reg & BIT_TX_DITHER)
            {
                HI_DRV_PROC_EchoHelper("dither : Open \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("dither : close \n");
            }

            if(u32Reg & BIT_TX_DEMUX_YC )
            {
                HI_DRV_PROC_EchoHelper("One- to Two Demux : Open \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("One- to Two Demux : close \n");
            }

        }
        else if(!HI_OSAL_Strncmp(chArg2,"ro",DEF_FILE_NAMELENGTH))
        {
            HI_DRV_PROC_EchoHelper("not supported... \n");          
        }
        else if(!HI_OSAL_Strncmp(chArg2,"phy",DEF_FILE_NAMELENGTH))
        {
            // unknown DM_TX_CTRL2
            SI_TX_PHY_ReadRegister(0x06,&u32Reg);
            if(u32Reg != 0x89)
            {
                HI_DRV_PROC_EchoHelper("Phy Reg 0x06 : 0x%02x(0x89) \n",u32Reg);
            }

            // unknown DM_TX_CTRL3
            SI_TX_PHY_ReadRegister(0x07,&u32Reg);
            if(u32Reg != 0x81)
            {
                HI_DRV_PROC_EchoHelper("Phy Reg 0x07 : 0x%02x(0x81) \n",u32Reg);
            }

            // unknown DM_TX_CTRL5
            SI_TX_PHY_ReadRegister(0x09,&u32Reg);
            HI_DRV_PROC_EchoHelper("Phy Reg 0x09 : 0x%02x \n",u32Reg);

            // unknown BIAS_GEN_CTRL1 
            SI_TX_PHY_ReadRegister(0x0a,&u32Reg);
            if(u32Reg != 0x07)
            {
                HI_DRV_PROC_EchoHelper("Phy Reg 0x0a : 0x%02x(0x07) \n",u32Reg);
            }

            // unknown BIAS_GEN_CTRL2
            SI_TX_PHY_ReadRegister(0x0b,&u32Reg);
            if(u32Reg != 0x51)
            {
                HI_DRV_PROC_EchoHelper("Phy Reg 0x0b : 0x%02x(0x51) \n",u32Reg);
            }

            //unknown DM_TX_CTRL4 
            SI_TX_PHY_ReadRegister(0x08,&u32Reg);
            if(u32Reg != 0x40)
            {
                HI_DRV_PROC_EchoHelper("Phy Reg 0x08 : 0x%02x(0x40) \n",u32Reg);
            }

            // enc_bypass == nobypass
            SI_TX_PHY_ReadRegister(0x0d,&u32Reg);
            if(u32Reg != 0x00)
            {
                HI_DRV_PROC_EchoHelper("Phy Reg 0x0d : 0x%02x(0x00) \n",u32Reg);
            }

                     
            // term_en && cap_ctl  // term_en �ȹص�
            SI_TX_PHY_ReadRegister(0x0e,&u32Reg);
            if(u32Reg & 0x01)
            {
                HI_DRV_PROC_EchoHelper("term_en : Enable  \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("term_en : Disable  \n");
            }
            
            if(u32Reg & 0x02)
            {
                HI_DRV_PROC_EchoHelper("cap_ctl : Enable (recommond to disable)  \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("cap_ctl : Disable  \n");
            }

            // pll ctrl -deep color
            SI_TX_PHY_ReadRegister(0x02,&u32Reg);
            if((u32Reg & 0x03) == 0x00)
            {
                HI_DRV_PROC_EchoHelper("dpcolor_ctl  : 8bit  \n");
            }            
            else if((u32Reg & 0x03) == 0x01)
            {
                HI_DRV_PROC_EchoHelper("dpcolor_ctl  : 10bit  \n");
            }            
            else if((u32Reg & 0x03) == 0x02)
            {
                HI_DRV_PROC_EchoHelper("dpcolor_ctl  : 12bit  \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("dpcolor_ctl  : invalid  \n");
            }

            // oe && pwr_down
            SI_TX_PHY_ReadRegister(0x05,&u32Reg);
            if(u32Reg & 0x10)
            {
                HI_DRV_PROC_EchoHelper("Phy No power Down \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("Phy will be power down \n");
            }

            if(u32Reg & 0x20)
            {
                HI_DRV_PROC_EchoHelper("Phy Outputs enable\n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("Phy Outputs Disable\n");
            }         

            SI_TX_PHY_ReadRegister(0x0c,&u32Reg);
            if(u32Reg & 0x01)
            {
                HI_DRV_PROC_EchoHelper("receiver is connected \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("receiver is dis-connected \n");
            }

            if(u32Reg & 0x02)
            {
                HI_DRV_PROC_EchoHelper("Clock detected > 2.5Mhz \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("No clock detected \n");
            }      
                        
        }
    }  
    else if(!HI_OSAL_Strncmp(chArg1,"fmtforce",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("Default Fmt Delay\n");
            SetForceDelayMode(HI_FALSE,IsForceMuteDelay());
        }
        else if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("Force Fmt Delay\n");
            SetForceDelayMode(HI_TRUE,IsForceMuteDelay());
        }              
    }
    else if(!HI_OSAL_Strncmp(chArg1,"muteforce",DEF_FILE_NAMELENGTH))
    {
        if(chArg2[0] == '0')
        {
            HI_DRV_PROC_EchoHelper("Default mute Delay\n");
            SetForceDelayMode(IsForceFmtDelay(),HI_FALSE);
        }
        else if(chArg2[0] == '1')
        {
            HI_DRV_PROC_EchoHelper("Force Delay\n");
            SetForceDelayMode(IsForceFmtDelay(),HI_TRUE);
        }                      
    }
    else if(!HI_OSAL_Strncmp(chArg1,"fmtdelay",DEF_FILE_NAMELENGTH))
    {
        HI_U32 delay;
        
        delay = simple_strtol(chArg2, NULL, 10);
            
        if(delay <= 10000)
        {
            HI_DRV_PROC_EchoHelper("Set Foramt Delay %d ms\n",delay);
            SetGlobalFmtDelay(delay);
        }
        else
        {
            HI_DRV_PROC_EchoHelper("Out of range[0-10000] %d ms\n",delay); 
        }              
    }
    else if(!HI_OSAL_Strncmp(chArg1,"mutedelay",DEF_FILE_NAMELENGTH))
    {
        HI_U32 delay;
        
        delay = simple_strtol(chArg2, NULL, 10);
            
        if(delay <= 10000)
        {
            HI_DRV_PROC_EchoHelper("Avmut Delay %d ms\n",delay);
            SetGlobalMuteDelay(delay);
        }
        else
        {
            HI_DRV_PROC_EchoHelper("Out of range[0-10000] %d ms\n",delay); 
        }              
    }
    else if(!HI_OSAL_Strncmp(chArg1,"oe",DEF_FILE_NAMELENGTH))
    {
        if(SI_TX_PHY_GetOutPutEnable())
        {
            //DRV_HDMI_SetAVMute(0,HI_TRUE);
            SI_TX_PHY_DisableHdmiOutput();
        }
        else
        {
            SI_TX_PHY_EnableHdmiOutput();
            //DRV_HDMI_SetAVMute(0,HI_FALSE);
        }
    }
    else if(!HI_OSAL_Strncmp(chArg1,"ddc",DEF_FILE_NAMELENGTH))
    {
        HI_U32 delay;
        
        delay = simple_strtol(chArg2, NULL, 10);
            
        if(delay <= 127)
        {
            if(delay != 0)
            {
                HI_DRV_PROC_EchoHelper("DDC Speed nearly %d hz\n",(2000000 / delay));
            }
            else
            {
                HI_DRV_PROC_EchoHelper("DDC Speed 0 hz\n");
            }
            
            DRV_Set_DDCSpeed(delay);
        }
        else
        {
            HI_DRV_PROC_EchoHelper("Delay Count Out of range[0-127] %d\n",delay); 
        }              
    }
    else if(!HI_OSAL_Strncmp(chArg1,"edid",DEF_FILE_NAMELENGTH))
    {
        HI_U32 edidNum = DRV_Get_DebugEdidNum();
        HI_U32 edidIndex = 0;
        
        edidIndex = simple_strtol(chArg2, NULL, 10);
            
        if((edidIndex > 0) && (edidIndex <= edidNum))
        {
            HDMI_EDID_S stEDID;
            HDMI_Test_EDID_S *pEdidTmp = DRV_Get_DebugEdid(edidIndex);



            DRV_Set_UserEdidMode(HI_UNF_HDMI_ID_0,HI_TRUE);
            
            memset(&stEDID,0,sizeof(HDMI_EDID_S));
            memcpy(stEDID.u8Edid,pEdidTmp->u8Edid,pEdidTmp->bEdidLen);
            stEDID.u32Edidlength = pEdidTmp->bEdidLen;
            
            DRV_Set_UserEdid(HI_UNF_HDMI_ID_0, &stEDID);

            HI_DRV_PROC_EchoHelper("load UserSetting EDID:%d success,please swrst \n",edidIndex);
        }
        else
        {
            if(edidIndex == 0)
            {
                DRV_Set_UserEdidMode(HI_UNF_HDMI_ID_0,HI_FALSE);          
                HI_DRV_PROC_EchoHelper("Stop UserSettting EDID Mode \n");
            }
            else
            {
                HI_DRV_PROC_EchoHelper("Index:[%d] Out of Edid Range [%d] \n",edidIndex,edidNum);
            }
            
            HI_DRV_PROC_EchoHelper("----------------------- edidIndex %02d Out of range[1-%02d] ---------------------------\n",edidIndex,edidNum);
            HI_DRV_PROC_EchoHelper("01. normal test\n");
            HI_DRV_PROC_EchoHelper("02. for edid test block 4\n");
            HI_DRV_PROC_EchoHelper("03. alter from edid test block 4,ext num err,and vsdb in block 3\n"); 
            HI_DRV_PROC_EchoHelper("04. extend version 2.4\n");
            HI_DRV_PROC_EchoHelper("05. unknown,maybe test parse edid some function\n");
            HI_DRV_PROC_EchoHelper("06. audio Amplifier 1 \n"); 
            HI_DRV_PROC_EchoHelper("07. ony dvi device, extend 2.1 detailed timing \n");
            HI_DRV_PROC_EchoHelper("08. extend num && header err\n");
            HI_DRV_PROC_EchoHelper("09. unknown \n"); 
            HI_DRV_PROC_EchoHelper("10. audio Amplifier 2 \n");
            HI_DRV_PROC_EchoHelper("11. hdcp test quantumdata HDMI \n");
            HI_DRV_PROC_EchoHelper("12. hdcp test quantumdata DVI \n"); 
            HI_DRV_PROC_EchoHelper("13. hitachi no vsdb(in some tv,some byte has broken) && block 2 crc err && support audio \n");
            HI_DRV_PROC_EchoHelper("14. all 0xff and 0x7f,err edid, can read 2 block \n");
            HI_DRV_PROC_EchoHelper("15. all 0xff and 0x7f,err edid, can read 4 block \n"); 
            HI_DRV_PROC_EchoHelper("16. 2block 1block ok,2block all 0xff(0x7f) \n");
            HI_DRV_PROC_EchoHelper("17. 4block 1block ok,2/3/4block all 0xff(0x7f) \n");
            HI_DRV_PROC_EchoHelper("18. 2block and 1st block header all 0xff,crc ok \n"); 
            HI_DRV_PROC_EchoHelper("19. dvi 1 block 0 extend ; crc ok \n");
            HI_DRV_PROC_EchoHelper("20. dvi 1 block extend != 0 ; crc error \n");
            HI_DRV_PROC_EchoHelper("21. dvi 2 block crc ok whole detailed in block 2 \n"); 
            HI_DRV_PROC_EchoHelper("22. dvi 2 block crc err \n");
            HI_DRV_PROC_EchoHelper("23. customer header err, base&extend crc err, extend version err \n");
            HI_DRV_PROC_EchoHelper("-------------------------------------------------------------------------------------\n");
        }

        HI_DRV_PROC_EchoHelper("swrst for trigle hotplug... ... ... \n");
        SI_SW_ResetHDMITX();
    }
    else 
    {
        HI_DRV_PROC_EchoHelper("--------------------------------- HDMI debug options --------------------------------\n");                                                     
        HI_DRV_PROC_EchoHelper("you can perform HDMI debug with such commond:\n");                                                                      
        HI_DRV_PROC_EchoHelper("echo [arg1] [arg2] > /proc/msp/hdmi \n\n");                                                                             
        HI_DRV_PROC_EchoHelper("debug action                      arg1         arg2\n");                                                                
        HI_DRV_PROC_EchoHelper("------------------------------    --------    ---------------------------------------\n");                                                
        HI_DRV_PROC_EchoHelper("colorbar                          cbar        0 disable / 1 enable \n");
        HI_DRV_PROC_EchoHelper("vblank(yuv data from hdmi)        vblank      0 /red / green/ blue/ black \n");
        HI_DRV_PROC_EchoHelper("DVI encoder                       enc         phy(default)/ctrl  \n");
        HI_DRV_PROC_EchoHelper("pixel clk bypass                  pclk        0 nobypass(default) 1 bypass  \n");
        HI_DRV_PROC_EchoHelper("software reset                    swrst       no param \n"); 
        HI_DRV_PROC_EchoHelper("invert Tmds clk                   tclk        0 not invert(default) / 1 invert \n");  
        HI_DRV_PROC_EchoHelper("Avmute                            mute        0 unmute/ 1 mute \n");
        HI_DRV_PROC_EchoHelper("Set 3D InfoFrame                  3d          0 disable3D /fp/sbs/tab  \n");
        HI_DRV_PROC_EchoHelper("Debug audio Change                audio       0 I2S / 1 SPdif / 2 HBR   \n");
        HI_DRV_PROC_EchoHelper("Thread stop/start                 thread      0 stop / 1 start  \n");
        HI_DRV_PROC_EchoHelper("cec enable                        cec         0 disable / 1 enable \n");
        HI_DRV_PROC_EchoHelper("Force set attr                    setAttr     no param \n"); 
        HI_DRV_PROC_EchoHelper("check                             check       timing / ddc / color /ro (read only regs) / phy\n"); 
        HI_DRV_PROC_EchoHelper("Force SetForamt Delay             fmtforce    0 Default / 1 Force \n"); 
        HI_DRV_PROC_EchoHelper("Force Avmute Delay                muteforce   0 Default / 1 Force \n"); 
        HI_DRV_PROC_EchoHelper("Set Foramt Delay                  fmtdelay    0 ~ 10000 (ms)\n"); 
        HI_DRV_PROC_EchoHelper("Avmute Delay                      mutedelay   0 ~ 10000 (ms)\n"); 
        HI_DRV_PROC_EchoHelper("Phy OE open/close                 oe          no param\n"); 
        HI_DRV_PROC_EchoHelper("Change DDC SPEED                  ddc         0 ~ 127 (n=0:0khz  n=1-127: 2000/n khz)\n"); 
        HI_DRV_PROC_EchoHelper("Debug edid,need swrst             edid        1 ~ %d / 0(list && stop) \n",DRV_Get_DebugEdidNum()); 
        HI_DRV_PROC_EchoHelper("-------------------------------------------------------------------------------------\n"); 
    }
#endif
    return count;
}


HI_S32 HDMI_ModeInit(HI_VOID)
{
    DRV_PROC_ITEM_S  *pProcItem;

    DRV_PROC_EX_S stFnOpt =
    {
         .fnRead = HDMI0_Proc,
    };

    DRV_PROC_EX_S stFnSinkOpt =
    {
         .fnRead = HDMI0_Sink_Proc,
    };

    /* Register hdmi device */
    //sprintf(g_hdmiRegisterData.devfs_name, UMAP_DEVNAME_HDMI);
    HI_OSAL_Snprintf(g_hdmiRegisterData.devfs_name, 64, UMAP_DEVNAME_HDMI);
    g_hdmiRegisterData.fops   = &hdmi_FOPS;
    g_hdmiRegisterData.drvops = &hdmi_DRVOPS;
    g_hdmiRegisterData.minor  = UMAP_MIN_MINOR_HDMI;
    g_hdmiRegisterData.owner  = THIS_MODULE;
    if (HI_DRV_DEV_Register(&g_hdmiRegisterData) < 0)
    {
        HI_FATAL_HDMI("register hdmi failed.\n");
        return HI_FAILURE;
    }
    /* Register Proc hdmi Status */
    pProcItem = HI_DRV_PROC_AddModule("hdmi0", &stFnOpt, NULL);
    if(pProcItem != HI_NULL)
    {
        pProcItem->write = hdmi_ProcWrite;
    }

    //pProcItem = HI_DRV_PROC_AddModule("hdmi0_sink", &stFnSinkOpt, NULL);
    HI_DRV_PROC_AddModule("hdmi0_sink", &stFnSinkOpt, NULL);
    return HI_SUCCESS;
}

extern HI_S32  HDMI_DRV_Init(HI_VOID);


HI_VOID HDMI_ModeExit(HI_VOID)
{
    /* Unregister hdmi device */
    HI_DRV_PROC_RemoveModule("hdmi0");
    HI_DRV_PROC_RemoveModule("hdmi0_sink");
    HI_DRV_DEV_UnRegister(&g_hdmiRegisterData);
    return;
}

HI_S32 DRV_HDMI_Register(HI_VOID)
{
	HI_S32 ret;
	ret = HI_DRV_MODULE_Register((HI_U32)HI_ID_HDMI,HDMI_NAME,(HI_VOID *)&s_stHdmiExportFuncs);
    if (HI_SUCCESS != ret)
    {
        HI_FATAL_HDMI("HI_DRV_MODULE_Register failed\n");
        return ret;
    }
	return HI_SUCCESS;
}
HI_S32 DRV_HDMI_UnRegister(HI_VOID)
{
	HI_S32 ret;
	ret = HI_DRV_MODULE_UnRegister((HI_U32)HI_ID_HDMI);
    if (HI_SUCCESS != ret)
    {        
        HI_FATAL_HDMI("HI_DRV_MODULE_UnRegister failed\n");
        return ret;
    }
	return HI_SUCCESS;
}
int HDMI_DRV_ModInit(void)
{
    HI_S32 ret;
    
#if defined (SUPPORT_FPGA)
    SocHdmiInit();
#endif
    HDMI_ModeInit();

    ret = HDMI_DRV_Init();
    
#ifdef ANDROID_SUPPORT
    //android only
	if (switch_dev_register(&hdmi_tx_sdev))
    {
		HI_WARN_HDMI("\n Warning:! registering HDMI switch device Failed \n");		
		g_switchOk = HI_FALSE;
        //return -EINVAL;
	}
    else
    {
        g_switchOk = HI_TRUE;
    }
#endif

#ifdef MODULE
	HI_PRINT("Load hi_hdmi.ko success.\t(%s)\n", VERSION_STRING);
#endif

    return ret;
}

extern HI_VOID  HDMI_DRV_EXIT(HI_VOID);
void HDMI_DRV_ModExit(void)
{
    HI_U32 hdmiStatus;
    hdmiStatus = DRV_HDMI_GetInitNum(HI_UNF_HDMI_ID_0);
    if(hdmiStatus > 0)
    {
        HI_DRV_HDMI_Close(HI_UNF_HDMI_ID_0);
    }

    HDMI_ModeExit();
         
    HDMI_DRV_EXIT();
    //HI_DRV_HDMI_PlayStus(HI_UNF_HDMI_ID_0,&temp);
    //if(temp == HI_TRUE)
    //{
        //HI_DRV_HDMI_Close(HI_UNF_HDMI_ID_0);
    //}
#ifdef ANDROID_SUPPORT
    //android ����
    if(g_switchOk == HI_TRUE)
    {
    	switch_dev_unregister(&hdmi_tx_sdev);
    }
#endif

#ifdef MODULE
    HI_PRINT("remove hi_hdmi.ko success.\t(%s)\n", VERSION_STRING);
#endif

    return;
}

module_init(HDMI_DRV_ModInit);
module_exit(HDMI_DRV_ModExit);
MODULE_LICENSE("GPL");




