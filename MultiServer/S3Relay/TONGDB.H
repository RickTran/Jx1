// TongDB.h: interface for the CTongDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TONGDB_H__8C487F07_C490_4562_AD01_77953CCCBCE2__INCLUDED_)
#define AFX_TONGDB_H__8C487F07_C490_4562_AD01_77953CCCBCE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DBTable.h"
#include "KTongControl.h"

#define MAX_TONG_COUNT 10000

/*
enum enumMemberClass		//��������
{
	mcMaster = 0,			//����
	mcDirector,			//����
	mcManager,			//����
	mcMember			//����
};
*/

typedef struct
{
	int			nCamp;								// �����Ӫ
	DWORD		dwMoney;								// ����ʽ�
	int			nCredit;								// �������
	int			nLevel;								// ���ȼ�

	char		szName[defTONG_STR_LENGTH];			// �����
	char		szPassword[defTONG_STR_LENGTH];		// �������

	char		szMasterTitle[defTONG_STR_LENGTH];	// �����ƺ�
	char		szDirectorTitle[defTONG_MAX_DIRECTOR][defTONG_STR_LENGTH];// �����ϳƺ�
	char		szManagerTitle[defTONG_MAX_MANAGER][defTONG_STR_LENGTH];	// ���ӳ��ƺ�
	char		szNormalTitle[defTONG_STR_LENGTH];	// �����ڳƺ�
}TTongStruct;	//�����������ݿ�Ľṹ

typedef struct
{
	char		szName[defTONG_STR_LENGTH];			// ������
	char		szTong[defTONG_STR_LENGTH];		// �����
	TONG_MEMBER_FIGURE		MemberClass;			//��������
	int			nTitleIndex;					//���ڳƺ�����
}TMemberStruct;	//�����������ݿ�Ľṹ

class CTongDB  
{
private:
	ZDBTable* m_TongTable;						//������ݱ�
	ZDBTable* m_MemberTable;					//�������ݱ�
	
	TTongList m_TongList[MAX_TONG_COUNT];	//����б�
	int m_TongListCount;						//����б���
public:
	CTongDB();
	virtual ~CTongDB(){};

	bool Open();
	void Close();
	
	bool ChangeTong(CTongControl& aTongControl);	//�����͸ı����
	bool DelTong(char* aTongName);	//ɾ������
	bool SearchTong(char* aTongName, CTongControl& aTongControl);	//�����ݿ���ȡ�ð���
	int GetTongCount();							//ȡ��������
	int GetTongList(TTongList* aTongList, int aMaxNum);	//ȡ�ð���б�

	bool ChangeMember(TMemberStruct aMember);//���Ӱ���
	bool DelMember(char* aMemberName);		//ɾ������
	
};

#endif // !defined(AFX_TONGDB_H__8C487F07_C490_4562_AD01_77953CCCBCE2__INCLUDED_)