/****************************************************
Copyright (C), 1988-2009, Huawei Tech. Co., Ltd.
File name:      pulldown.h
Author:   z00128410  Version:   1.0     Date: 2009-1-6
Description:    ���ļ���Ҫ�����pulldown���ģ������Ҫ
�õ��Ľṹ�����Ͷ��塢�궨�塢����ԭ�͵Ķ��塣
pulldown���ģ��pulldown.cpp����Ҫ�������ļ���
Others:         // �������ݵ�˵��
Function List:  // ��Ҫ�����б�ÿ����¼Ӧ���������������ܼ�Ҫ˵��
1. ....
History:        // �޸���ʷ��¼�б�ÿ���޸ļ�¼Ӧ�����޸����ڡ��޸�
// �߼��޸����ݼ���  
1. Date:
Author:
Modification:
2. ...
****************************************************/


#ifndef __PULLDOWN_HEADER__
#define __PULLDOWN_HEADER__

#define FIELD_BOTTOM_FIRST  0
#define FIELD_TOP_FIRST     1
#define FIELD_UNKNOWN       2

#define MyAssert(x) { if (!(x)) { printf("\nErr @ (%s, %d)\n", __FILE__, __LINE__); \
exit(-__LINE__); } }


//�������õĶ��׳�������ֵ�ṹ�壺 PULLDOWN_INPUT_CNT_S
typedef struct
{
    int FC_T;  
    int SC_T;  
    int TC_T;
    int FC_B;
    int SC_B;
    int TC_B;
    int z14;
    int z32;
    int z34;
    int mbNum;
}PULLDOWN_INPUT_CNT_S;

//3:2�������ˮ�߽ṹ�壺Pdown32Thd
typedef struct
{
    int FThresh;
    int SThresh;
    int TThresh;
}Pdwn32Thd;

//13:12�������ˮ�߽ṹ�壺Pdown1312Thd
typedef struct 
{
	int FThresh;
	int SThresh;
	int TThresh;
	int zthr;
}Pdwn1312Thd;


//ö������Pattern����ʾ����Ϸ�ʽ
typedef enum
{Tc = 0, Bc, TcBc, TcBl, TcBn, BcTl, BcTn} 
PATTERN_LIST;

//��������ṹ��PULLDOWN_OUTPUT_PARAM_S
typedef struct
{
	int Fieldorder;      //��ʾ����
    int CopyTime;        //��ʾ��ǰ���Ѿ����ֵĴ���   
	PATTERN_LIST Pattern;//��ʾ��ǰ������һ����Ͽ������һ֡
}PULLDOWN_OUTPUT_PARAM_S;


//���3:2�����м����ݱ�����CTXT32
typedef struct
{
    int frame_cnt;
    int pdcnt;
	int btmode;
    int flg;
    int FV_c;  
    int SV_c;  
    int TV_c;  
    int FV_p;  
    int SV_p;  
    int TV_p;	
    int FM[5]; 
    int SM[5]; 
    int TM[5]; 
}CTXT32;

//���1312�����м����ݱ�����CTXT1312
typedef struct  
{
	int frame_cnt;
    int pdcnt;
	int btmode;
    int flg;
    int FV_c;  
    int SV_c;  
    int TV_c;  
    int FV_p;  
    int SV_p;  
    int TV_p; 
    int z14;
    int z32;
    int z34;	
    int FM[25]; 
    int SM[25]; 
    int TM[25];
}CTXT1312;

//��ѯ���ݽṹ�壬��������
typedef struct 
{
	int FC_T;
	int SC_T;
	int TC_T;
	int FC_B;
	int SC_B;
	int TC_B;
	int z14;
	int z32;
	int z34;
	int PD32flg_T;
	int PD32flg_B;
	int PD32cnt_T;
	int PD32cnt_B;
	int PD32btmode;
	int PD1312flg_T;
	int PD1312flg_B;
	int PD1312cnt_T;
	int PD1312cnt_B;
	int PD1312btmode;
}PULLDOWN_PRIVATE_DATA_S;


//��ʼ������
void InitPulldown(int mbNum);
//�����㺯��
void CalcPulldown(PULLDOWN_INPUT_CNT_S pdownInputCnt, PULLDOWN_OUTPUT_PARAM_S *ppdownOutputTop,
				  PULLDOWN_OUTPUT_PARAM_S *ppdownOutputBtm);
//��ѯ�м����ݺ�������ӡ��Ϣ
void InspectPlldwnData(PULLDOWN_PRIVATE_DATA_S *pPrivateData);

#endif
