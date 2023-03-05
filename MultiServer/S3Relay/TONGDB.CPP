// TongDB.cpp: implementation of the CTongDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "db.h"
#include "DBTable.h"
#include "TongDB.h"

CTongDB		g_cTongDB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
int get_TongName(DB *db, const DBT *pkey, const DBT *pdata, DBT *ikey) 
{
	//����һ��������buffer���õ�account��Ϊ����
	memset( ikey, 0, sizeof( DBT ) );
	//TRoleData *pRoleData = (TRoleData *)pdata->data;
	//ikey->data = pRoleData->BaseInfo.caccname;
	//ikey->size = strlen( pRoleData->BaseInfo.caccname ) + 1;
	TMemberStruct* pMemberStruct = (TMemberStruct*)pdata->data;
	ikey->data = pMemberStruct->szTong;
	ikey->size = strlen( pMemberStruct->szTong ) + 1;
	return 0;
}

CTongDB::CTongDB()
{//��ʼ������
	m_TongListCount = 0;
	memset(m_TongList,0,sizeof(TTongList) * MAX_TONG_COUNT);
}

bool CTongDB::Open()
{
	m_TongTable = new ZDBTable("TongDB","TongTable");		//�򿪰���
	if(!m_TongTable->open())return false;

	m_MemberTable = new ZDBTable("TongMemberDB","MemberTable");	//���ڱ�
	m_MemberTable->addIndex(get_TongName);
	if(!m_MemberTable->open())
	{
		m_TongTable->close();
		return false;
	}
	return true;
}
void CTongDB::Close()
{
	m_TongTable->close();
	m_MemberTable->close();
	delete m_TongTable;
	delete m_MemberTable;
}

bool CTongDB::ChangeTong(CTongControl& aTongControl)
{//�����͸ı����
	bool aResult = true;
	int aKeySize = strlen(aTongControl.m_szName) + 1;
	int aSize = sizeof(TTongStruct);
	TTongStruct aTongStruct;
	memset(&aTongStruct, 0, aSize);
	aTongStruct.nCamp = aTongControl.m_nCamp;
	aTongStruct.dwMoney = aTongControl.m_dwMoney;
	aTongStruct.nCredit = aTongControl.m_nCredit;
	aTongStruct.nLevel = aTongControl.m_nLevel;
	strcpy(aTongStruct.szName, aTongControl.m_szName);
	strcpy(aTongStruct.szMasterTitle, aTongControl.m_szMasterTitle);
	strcpy(aTongStruct.szNormalTitle, aTongControl.m_szNormalTitle);
	int i;
	for(i=0;i<defTONG_MAX_DIRECTOR;++i)
	{
		strcpy(aTongStruct.szDirectorTitle[i], aTongControl.m_szDirectorTitle[i]);
	}
	for(i=0;i<defTONG_MAX_MANAGER;++i)
	{
		strcpy(aTongStruct.szManagerTitle[i], aTongControl.m_szManagerTitle[i]);
	}
	aResult = m_TongTable->add(aTongControl.m_szName, aKeySize,(char*)(&aTongStruct), aSize);

	return aResult;
}

bool CTongDB::DelTong(char* aTongName)
{//ɾ������
	bool aResult = true;
	int aKeySize = strlen(aTongName) + 1;

	aResult = m_TongTable->remove(aTongName, aKeySize);
	if(!aResult) return aResult;

	while(!m_MemberTable->remove(aTongName, aKeySize, 0))
		{}
	return true;
}

bool CTongDB::SearchTong(char* aTongName, CTongControl& aTongControl)
{//�����ݿ���ȡ�ð���
	bool aResult = true;
	int aSize;
	int aKeySize = strlen(aTongName) + 1;
	char* aBuffer = m_TongTable->search(aTongName, aKeySize, aSize);
	if(!aBuffer) return false;	//�Ҳ�������

	TTongStruct* pTongStruct = (TTongStruct*)aBuffer;
	aTongControl.m_nCamp = pTongStruct->nCamp;
	strcpy(aTongControl.m_szName, pTongStruct->szName);
	strcpy(aTongControl.m_szPassword, pTongStruct->szPassword);
	strcpy(aTongControl.m_szMasterTitle, pTongStruct->szMasterTitle);
	strcpy(aTongControl.m_szNormalTitle, pTongStruct->szNormalTitle);
	int i;
	for(i=0;i<defTONG_MAX_DIRECTOR;++i)
	{
		strcpy(aTongControl.m_szDirectorTitle[i], pTongStruct->szDirectorTitle[i]);
	}
	for(i=0;i<defTONG_MAX_MANAGER;++i)
	{
		strcpy(aTongControl.m_szManagerTitle[i], pTongStruct->szManagerTitle[i]);
	}

	int aDirectorNum = 0;
	int aManagerNum = 0;
	int aMemberNum = 0;

	aBuffer = m_MemberTable->search(aTongName, aKeySize, aSize, 0);
	while(aBuffer)
	{
		TMemberStruct* pMemberStruct = (TMemberStruct*)aBuffer;
		switch(pMemberStruct->MemberClass)
		{
		case enumTONG_FIGURE_MASTER:
			strcpy(aTongControl.m_szMasterName, pMemberStruct->szName);
			aTongControl.m_dwMasterID = g_String2Id(pMemberStruct->szName);
			break;
			
		case enumTONG_FIGURE_DIRECTOR:
			if (pMemberStruct->nTitleIndex < 0 || pMemberStruct->nTitleIndex >= defTONG_MAX_DIRECTOR)
				break;
			strcpy(aTongControl.m_szDirectorName[pMemberStruct->nTitleIndex], pMemberStruct->szName);
			aTongControl.m_dwDirectorID[pMemberStruct->nTitleIndex] = g_String2Id(pMemberStruct->szName);
			aDirectorNum++;
			break;
			
		case enumTONG_FIGURE_MANAGER:
			if (pMemberStruct->nTitleIndex < 0 || pMemberStruct->nTitleIndex >= defTONG_MAX_MANAGER)
				break;
			strcpy(aTongControl.m_szManagerName[pMemberStruct->nTitleIndex], pMemberStruct->szName);
			aTongControl.m_dwManagerID[pMemberStruct->nTitleIndex] = g_String2Id(pMemberStruct->szName);
			aManagerNum++;
			break;
			
		case enumTONG_FIGURE_MEMBER:
			aTongControl.AddMember(pMemberStruct->szName);
			aMemberNum++;
			break;
			
		}
		aBuffer = m_MemberTable->next(aSize);
	}
	aTongControl.m_nDirectorNum = aDirectorNum;
	aTongControl.m_nManagerNum = aManagerNum;
	aTongControl.m_nMemberNum = aMemberNum;

	delete [] aBuffer;

	return aResult;
}

int CTongDB::GetTongCount()
{//ȡ��������
	int aCount = 0;
	int aTongSize;
	int aTongKeySize;
	char aTongKeyBuffer[defTONG_STR_LENGTH];
	char aTongBuffer[sizeof(TTongStruct)];
	bool aResult = m_TongTable->GetRecordEx(aTongBuffer, aTongSize, aTongKeyBuffer, aTongKeySize,ZDBTable::cpFirst);
	while(aResult)
	{
		aCount++;
		aResult = m_TongTable->GetRecordEx(aTongBuffer, aTongSize, aTongKeyBuffer, aTongKeySize,ZDBTable::cpNext);
	}

	return aCount;
}

int CTongDB::GetTongList(TTongList* aTongList, int aMaxNum)
{//ȡ�ð���б�
		int aCount = 0;
	int aTongSize;
	int aTongKeySize;
	char aTongKeyBuffer[defTONG_STR_LENGTH];
	char aTongBuffer[sizeof(TTongStruct)];
	bool aResult = m_TongTable->GetRecordEx(aTongBuffer, aTongSize, aTongKeyBuffer, aTongKeySize,ZDBTable::cpFirst);
	while(aResult)
	{
		TTongStruct* pTong = (TTongStruct*)aTongBuffer;
		strcpy(aTongList[aCount].szName, pTong->szName);

		int aMemCount = 0;
		int aMemSize;
		char* aMemBuffer;
		aMemBuffer = m_MemberTable->search(aTongKeyBuffer, aTongKeySize, aMemSize, 0);
		while(aMemBuffer)
		{
			TMemberStruct* pMember = (TMemberStruct*)aMemBuffer;
			if(pMember->MemberClass == enumTONG_FIGURE_MEMBER)
					aMemCount++;
			delete [] aMemBuffer;
			aMemBuffer = m_MemberTable->next(aMemSize);
		}

		aTongList[aCount].MemberCount = aMemCount;

		aCount++;
		if( aCount>= aMaxNum ) break;
		aResult = m_TongTable->GetRecordEx(aTongBuffer, aTongSize, aTongKeyBuffer, aTongKeySize,ZDBTable::cpNext);
	}

	return aCount;
}

bool CTongDB::ChangeMember(TMemberStruct aMember)
{//���Ӱ���
	return m_MemberTable->add(aMember.szName, strlen(aMember.szName) + 1, (char*)(&aMember), sizeof(TMemberStruct));
}


bool CTongDB::DelMember(char* aMemberName)
{//ɾ������
	int aKeySize = strlen(aMemberName) + 1;

	return m_MemberTable->remove(aMemberName, aKeySize);
}