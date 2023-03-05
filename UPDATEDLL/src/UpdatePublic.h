//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   UpdatePublic.h
//  Version     :   1.0
//  Creater     :   Cheng Bitao
//  Date        :   2002-1-17 11:14:17
//  Comment     :   Define the comment interface of update system
//
//////////////////////////////////////////////////////////////////////////////////////


#ifndef __UPDATE_PUBLIC_H__
#define __UPDATE_PUBLIC_H__ 1

#include "DataDefine.h"
#include "ProcessIndex.h" 

extern CProcessIndex g_ProcessIndex;

//------------------------------------------------------------------------------------


int GetSerialNumberFromRegistry(HKEY hKey, const char cszKeyName[]);

int CheckSerialNumberValidity();

int GetInstallComponetInfo(const char cszIndexTxtFile[]);
int ProcessIndexFile(const char cszFileName[]);
int UpdateFiles();
int UpdateSelf();

int GetHostURL(const char cszFileName[], const char cszHostName[], char szHostURL[]);

///////////////////////////////////////////////////////////////////////////////
//���°��ϲ� Add By Fellow 2003.08.11
//һ��Pack�ļ���������Ľṹ:
//�������ĸ��ֽڵ��ļ���ͷ��־:�ַ���'PACK',Ȼ���������ĿȻ����������ʼ��ƫ����\���ݿ�ʼ��ƫ����,Ȼ����У���,Ȼ���Ǳ������ֽ�:
typedef struct {
	unsigned char signature[4];			//"PACK"
	unsigned long count;				//���ݵ���Ŀ��
	unsigned long index_offset;			//������ƫ����
	unsigned long data_offset;			//���ݵ�ƫ����
	unsigned long crc32;
	unsigned char reserved[12];
} t_pack_header;

typedef struct {		//������Ϣ
	unsigned long id;
	unsigned long offset;
	long size;
	long compress_size;
} t_index_info;
void GetHeader(t_pack_header* aHeader, FILE* aFile);
void GetIndexInfo(t_index_info* aIndexInfo, const t_pack_header* aHeader, FILE* aFile);
bool PackComb(const char* stdFilePath,const char* addFilePath);
///////////////////////////////////////////////////////////////////////////

#endif  //__UPDATE_PUBLIC_H__

