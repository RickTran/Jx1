//---------------------------------------------------------------------------
// Sword3 Core (c) 2002 by Kingsoft
//
// File:	KItemGenerator.CPP
// Date:	2002.08.26
// Code:	DongBo
// Desc:    CPP file. ���ļ�ʵ�ֵ����������ɵ���
//---------------------------------------------------------------------------
#include "KEngine.h"
#include "KCore.h"
#include "MyAssert.H"
#include "KItem.h"
#include "KSubWorldSet.h"
#include "KItemGenerator.h"

KItemGenerator	ItemGen;			//	װ��������

KItemGenerator::KItemGenerator()
{
}

KItemGenerator::~KItemGenerator()
{
}

/******************************************************************************
	���ܣ�	���ݳ�ʼ��. ��tab file�ж�ȡ����
******************************************************************************/
BOOL KItemGenerator::Init()
{
	if (!m_BPTLib.Init())		// �˵��ô����ɵ�tab file���������еĳ�ʼ����
		return FALSE;

	for (int i = equip_meleeweapon; i < equip_detailnum - equip_meleeweapon; i++)
	{
		switch(i)
		{
		case equip_meleeweapon:
			m_EquipNumOfEntries[i] = m_BPTLib.GetMeleeWeaponRecordNumber();
			break;
		case equip_rangeweapon:
			m_EquipNumOfEntries[i] = m_BPTLib.GetRangeWeaponRecordNumber();
			break;
		case equip_armor:
			m_EquipNumOfEntries[i] = m_BPTLib.GetArmorRecordNumber();
			break;
		case equip_helm:
			m_EquipNumOfEntries[i] = m_BPTLib.GetHelmRecordNumber();
			break;
		case equip_boots:
			m_EquipNumOfEntries[i] = m_BPTLib.GetBootRecordNumber();
			break;
		case equip_belt:
			m_EquipNumOfEntries[i] = m_BPTLib.GetBeltRecordNumber();
			break;
		case equip_cuff:
			m_EquipNumOfEntries[i] = m_BPTLib.GetCuffRecordNumber();
			break;
		case equip_amulet:
			m_EquipNumOfEntries[i] = m_BPTLib.GetAmuletRecordNumber();
			break;
		case equip_ring:
			m_EquipNumOfEntries[i] = m_BPTLib.GetRingRecordNumber();
			break;
		case equip_pendant:
			m_EquipNumOfEntries[i] = m_BPTLib.GetPendantRecordNumber();
			break;
		case equip_horse:
			m_EquipNumOfEntries[i] = m_BPTLib.GetHorseRecordNumber();
			break;
		}
	}
	m_MedNumOfEntries = m_BPTLib.GetMedicineRecordNumber();
	return TRUE;
/*
	m_BPTLib.Init();
	int naryMALevel[] = {1,2,3,4,5,6};
	KItem theItem;
	Gen_Equipment(1, 1, 0, 5, naryMALevel, 45, &theItem);
	return TRUE;
*/
}

/******************************************************************************
	���ܣ�	����ҩƷ
	���:	nDetailType: ҩƷ����
			nLevel: �ȼ�
			nVersion: �汾��
	����:	�ɹ�ʱ���ط���, ��������� pItem ��ָ�����и���
			ʧ��ʱ������
******************************************************************************/
BOOL KItemGenerator::Gen_Medicine(IN int nDetailType,
								  IN int nLevel,
								  IN int nVersion,
								  IN OUT KItem* pItem)
{
	_ASSERT(this != NULL);
	_ASSERT(pItem != NULL);

	BOOL bEC = FALSE;
	
#ifdef _SERVER	// �������汾
	pItem->m_GeneratorParam.uRandomSeed = g_GetRandomSeed();
	pItem->m_GeneratorParam.nVersion = g_SubWorldSet.GetGameVersion();
	pItem->m_GeneratorParam.nLuck = 0;
#else
	g_RandomSeed(pItem->m_GeneratorParam.uRandomSeed);
	int nGameVersion = pItem->m_GeneratorParam.nVersion;
#endif

	const int i = nDetailType * 5 + nLevel - 1;	// ����ʽ�ɲ߻�����
												// �ȼ����ݴ�1��ʼ������Ҫ������
	const KBASICPROP_MEDICINE* pMed = NULL;
	pMed = m_BPTLib.GetMedicineRecord(i);
	if (NULL == pMed)
	{
		_ASSERT(FALSE);
		return bEC;
	}
	*pItem = *pMed;
	return TRUE;
}
/******************************************************************************
	���ܣ�	�������װ��
	���:	nSeriesReq: ��������
			nLevel: �ȼ�
			nLucky: ����ֵ
			nVersion: �汾��
	����:	�ɹ�ʱ���ط���, ��������� pItem ��ָ�����и���
			ʧ��ʱ������
******************************************************************************/
BOOL KItemGenerator::Gen_Item(IN int nItemGenre,
							  IN int nSeriesReq,
							  IN int nLevel,
							  IN int nLucky,
							  IN OUT KItem* pItem)
{
	_ASSERT(this != NULL);
	_ASSERT(pItem != NULL);

	switch (nItemGenre)
	{
	case item_equip:
		{
			int nDetailType, nParticularType, i;
			nDetailType = g_Random(equip_detailnum);
			nParticularType = g_Random(m_EquipNumOfEntries[nDetailType] + 1);
			int naryMALevel[6];
			ZeroMemory(naryMALevel, 6 * sizeof(int));
			for (i = 0; i < 6; i++)
			{
				naryMALevel[i] = g_Random(nLevel / 10 + 1);
				if (!naryMALevel[i])
					break;
			}
			Gen_Equipment(nDetailType, nParticularType, nSeriesReq, nLevel, naryMALevel, nLucky, g_SubWorldSet.GetGameVersion(), pItem);
		}
		break;
	case item_medicine:
		{
			int nDetailType;
			nDetailType = g_Random(medicine_detailnum);
			Gen_Medicine(nDetailType, nLevel, g_SubWorldSet.GetGameVersion(), pItem);
		}
		break;
	case item_task:
		{
			Gen_Quest(nSeriesReq, pItem);			
		}
		break;
	case item_townportal:
		{
			Gen_TownPortal(pItem);
		}
		break;
	default:
		break;
	}
	
	return TRUE;
}
/******************************************************************************
	���ܣ�	�������װ��
	���:	nDetailType: �������, �����������������......
			nParticularType: ��ϸ���
			nSeriesReq: ��������
			nLevel: �ȼ�
			pnaryMALevel: ħ�����Եȼ�����[6]
			nLucky: ����ֵ
	����:	�ɹ�ʱ���ط���, ��������� pItem ��ָ�����и���
			ʧ��ʱ������
******************************************************************************/
BOOL KItemGenerator::Gen_Equipment(IN int nDetailType,
								   IN int nParticularType,
								   IN int nSeriesReq,
								   IN int nLevel,
								   IN const int* pnaryMALevel,
								   IN int nLucky,
								   IN int nVersion,
								   IN OUT KItem* pItem)
{
	_ASSERT(this != NULL);
	_ASSERT(pItem != NULL);

	BOOL bEC = FALSE;
#ifdef _SERVER	// �������汾
	pItem->m_GeneratorParam.uRandomSeed = g_GetRandomSeed();
	if (pnaryMALevel)
		memcpy(pItem->m_GeneratorParam.nGeneratorLevel, pnaryMALevel, sizeof(int) * 6);
	else
		ZeroMemory(pItem->m_GeneratorParam.nGeneratorLevel, sizeof(int) * 6);
	pItem->m_GeneratorParam.nVersion = g_SubWorldSet.GetGameVersion();
	pItem->m_GeneratorParam.nLuck = nLucky;
#else
	g_RandomSeed(pItem->m_GeneratorParam.uRandomSeed);
	if (pnaryMALevel)
		memcpy(pItem->m_GeneratorParam.nGeneratorLevel, pnaryMALevel, sizeof(int) * 6);
#endif
	int nGameVersion = pItem->m_GeneratorParam.nVersion;

	// ������ڲ���, ȷ��װ���Ļ�������
	const int i = nParticularType * 10 + nLevel - 1;// ����ʽ�ɲ߻�����
												// ���SPE 2002��8��31��7:40
												// email: ����������ֵ��
												// �ȼ����ݴ�1��ʼ������Ҫ������
	const KBASICPROP_EQUIPMENT* pEqu = NULL;
	switch(nDetailType)
	{
	case equip_meleeweapon:
		pEqu = m_BPTLib.GetMeleeWeaponRecord(i);
		break;
	case equip_rangeweapon:
		pEqu = m_BPTLib.GetRangeWeaponRecord(i);
		break;
	case equip_armor:
		pEqu = m_BPTLib.GetArmorRecord(i);
		break;
	case equip_helm:
		pEqu = m_BPTLib.GetHelmRecord(i);
		break;
	case equip_boots:
		pEqu = m_BPTLib.GetBootRecord(i);
		break;
	case equip_belt:
		pEqu = m_BPTLib.GetBeltRecord(i);
		break;
	case equip_amulet:
		pEqu = m_BPTLib.GetAmuletRecord(i);
		break;
	case equip_ring:
		pEqu = m_BPTLib.GetRingRecord(i);
		break;
	case equip_cuff:
		pEqu = m_BPTLib.GetCuffRecord(i);
		break;
	case equip_pendant:
		pEqu = m_BPTLib.GetPendantRecord(i);
		break;
	case equip_horse:
		pEqu = m_BPTLib.GetHorseRecord(i);
		break;
	default:
		break;
	}
	if (NULL == pEqu)
		{ _ASSERT(FALSE); return bEC; }
		// �������˵�ֱ��ԭ��: ֻ��n��װ��, ������ i ��ֵ��[0,n-1]֮��
		// ���3��: nParticularType ����?
		//			nLevel ����?
		//			ԭʼ��tab file������ m_BPTLib.m_BPTEquipment ��
		//			����������������?
	pItem->SetAttrib_CBR(pEqu);
	pItem->SetSeries(nSeriesReq);

	if (NULL == pnaryMALevel)
		return TRUE;
	// ȷ��װ����ħ������
	KItemNormalAttrib	sMA[6];	// ���ߵ�ħ������
	bEC = Gen_MagicAttrib(nDetailType, pnaryMALevel, nSeriesReq, nLucky, sMA, nGameVersion);
	if (bEC)
		pItem->SetAttrib_MA(sMA);

	return bEC;
}

/******************************************************************************
	���ܣ�	����ħ������
	���:	nType: װ������
			pnaryMALevel: ����ָ��, ����ħ���ĵȼ�Ҫ��, ��6��
			nSeriesReq: ��������
			nLucky: ����ֵ
	����:	�� pnaryMA ��ָ�����з���ħ�������Ժ�������, ��6��
******************************************************************************/
// Add by Freeway Chen in 2003.5.30
BOOL KItemGenerator::Gen_MagicAttrib(
    int nType, const int* pnaryMALevel, int nSeriesReq, int nLucky,
	KItemNormalAttrib* pnaryMA, int nGameVersion
)
{
    int nResult = false;

    KBPT_ClassMAIT  SelectedMagicTable;	
	// ��¼ÿһ����¼ѡ�е�ħ�����ԣ���Ҫ��Ϊ�����ʹ�ñ�־���
    KMAGICATTRIB_TABFILE *pMagicAttrTable[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

    int i = 0;

    _ASSERT(pnaryMALevel);
    _ASSERT(pnaryMA);

    for (i = 0; i < 6; i++) 
    {
        // ������ּ���Ϊ�㣬��ʾ����Ҫ���������ˣ��˳�
        if (pnaryMALevel[i] == 0)	
        	break;

        SelectedMagicTable.Clear();

        // ����ǰ׺��׺����Ʒ���͡������������󼶱����з���������ħ��������ȡ����
        const KBPT_ClassMAIT *pCMITItem = m_BPTLib.GetCMIT(
            1 - (i & 1),        // ǰ׺��׺
            nType,              // ��Ʒ����
            nSeriesReq,         // �������� 
            pnaryMALevel[i]     // ���󼶱�
        );

        _ASSERT(pCMITItem);
        if (!pCMITItem)
        {
            g_DebugLog(
                "[GenMagicAttrib] GetCMIT Error, nPrefixPostfix = %d, nType = %d, nSeries = %d, nLevel = %d", 
                1 - (i & 1), nType, nSeriesReq, pnaryMALevel[i]
            );
            break;
        }

        // ��������������ħ�����ԣ�����
        // 1.�Ƿ��Ѿ�ʹ�ã�
        // 2.����ֵ
        // 3.�Ƿ�����ͬ�ġ����Ե������
        // ȡ����Ӧ�������SelectedMagicTable��
        int nCMITItemCount = pCMITItem->GetCount();
        int j = 0;
        int nDecide = GetRandomNumber(0, 100 - 1) / (1 + nLucky * 20 / 100);

        for (j = 0; j < nCMITItemCount; j++)
        {
            // ��������ֵ��ȡ����Ӧ����
            int nMAIndex = pCMITItem->Get(j);
            KMAGICATTRIB_TABFILE *pMAItem = (KMAGICATTRIB_TABFILE *)m_BPTLib.GetMARecord(nMAIndex);
            if (!pMAItem)
            {
                _ASSERT(pMAItem);
                continue;
            }

            // ����Ѿ�ʹ�ã�����ȡ��һ��
            if (pMAItem->m_nUseFlag)
                continue;

            // ����������㣬ȡ��һ��
            if ((pMAItem->m_DropRate[nType]) <= nDecide)
                continue;
            
            // ����Ƿ��Ѿ�����ͬ�ġ����Ե������
            int k = 0;
            for (k = 0; k < i; k++)
            {
                _ASSERT(pMagicAttrTable[k]);

                if ((pMagicAttrTable[k]->m_MagicAttrib.nPropKind) == (pMAItem->m_MagicAttrib.nPropKind))
                    break;
            }

            // ������� �����Ե��������ͬ��˵���Ѿ���һ����ͬ��ֵ��ѡ�У���ôȡ��һ��
            if (k < i)
                continue;

            SelectedMagicTable.Insert(nMAIndex);
        }

        int nSelectedCount = SelectedMagicTable.GetCount();

        if (nSelectedCount == 0)
        {   
            // ���û������������ħ�����ԣ����˳�
            break;
        }
        
        // �����������������ħ�����ԣ�������
        int nLuckyItemIndex = GetRandomNumber(0, nSelectedCount - 1);
        KMAGICATTRIB_TABFILE *pMAItem = (KMAGICATTRIB_TABFILE *)m_BPTLib.GetMARecord(
             SelectedMagicTable.Get(nLuckyItemIndex)
        );

        // ��������������ħ�����ԣ��Ա�������ʱ��ʹ�ñ�־���
        pMagicAttrTable[i] = pMAItem;

        _ASSERT(pMAItem);
        _ASSERT(!(pMAItem->m_nUseFlag));     // ȷ����û��ʹ��

        // ����ʹ�ñ�־
        pMAItem->m_nUseFlag = true;
		
        pnaryMA[i].nAttribType = pMAItem->m_MagicAttrib.nPropKind;
        pnaryMA[i].nValue[0] = GetRandomNumber(
            pMAItem->m_MagicAttrib.aryRange[0].nMin, 
            pMAItem->m_MagicAttrib.aryRange[0].nMax
        );
        pnaryMA[i].nValue[1] = GetRandomNumber(
            pMAItem->m_MagicAttrib.aryRange[1].nMin, 
            pMAItem->m_MagicAttrib.aryRange[1].nMax
        );
        pnaryMA[i].nValue[2] = GetRandomNumber(
            pMAItem->m_MagicAttrib.aryRange[2].nMin, 
            pMAItem->m_MagicAttrib.aryRange[2].nMax
        );
		g_DebugLog(
			"[GenMagicAttrib] pnaryMA[%d]: (%d:%s), nAttribType = %d, nValue[0] = %d, nValue[1] = %d, nValue[2] = %d,", 
			i,
			SelectedMagicTable.Get(nLuckyItemIndex),
			pMAItem->m_szName,
			pnaryMA[i].nAttribType,
			pnaryMA[i].nValue[0],
			pnaryMA[i].nValue[1],
			pnaryMA[i].nValue[2]
		);

    } // for ������Ҫȡ�ص�ħ�����Ա�

    for (NULL; i < 6; i++)
    {
        // ���ʣ�µ���
        pnaryMA[i].nAttribType = 0;
        pnaryMA[i].nValue[0] = 0;
        pnaryMA[i].nValue[1] = 0;
        pnaryMA[i].nValue[2] = 0;
    }

    for (i = 0; i < 6; i++)
    {
        if (!pMagicAttrTable[i])
            break;

        _ASSERT(pMagicAttrTable[i]->m_nUseFlag);   // ȷ���Ѿ�ʹ��

        pMagicAttrTable[i]->m_nUseFlag = false;    // ����Ϊû��ʹ��
    }

    nResult = true;
//Exit0:
	return nResult;
}

// Following Code is implement by DongBo
//BOOL KItemGenerator::Gen_MagicAttrib(int nType, const int* pnaryMALevel, int nSeriesReq, int nLucky,
//									 KItemNormalAttrib* pnaryMA)
//{
//	
//    // ȷ�������ڸ�װ����ȫ��ħ�����Լ�������
//	if (FALSE == GMA_GetAvaliableMA(nType))
//		{ _ASSERT(FALSE); return FALSE; }
//
//	// ȷ��������ħ�����Եĵȼ�,���ָ���
//	if (FALSE == GMA_GetLevelAndDropRate(nType))
//		{ _ASSERT(FALSE); return FALSE; }
//
//	// ������ħ��������ѡ������������. �߻�Ҫ��ѡ��6��
//	for (int i = 0; i < 6; i++)				// ����: ǰ׺, ż��: ��׺
//	{
//		GMA_ChooseMA((i&1), pnaryMALevel[i], nLucky, &(pnaryMA[i]));
//		if (pnaryMA[i].nAttribType == 0)
//			break;
//	}
//	return TRUE;
//}

/******************************************************************************
	����:	ȷ��������ָ��װ����ȫ��ħ�����Լ�������
	���:	nType: ָ��װ��
	����:	�ɹ�ʱ���ط���. m_GMA_naryMA[0]���������ڸ�װ����ȫ��ħ������ǰ׺
							m_GMA_nCount[0]��������ħ�����Ե�����
							m_GMA_naryMA[1]���������ڸ�װ����ȫ��ħ�����Ժ�׺
							m_GMA_nCount[1]��������ħ�����Ե�����
			ʧ��ʱ������
******************************************************************************/
BOOL KItemGenerator::GMA_GetAvaliableMA(int nType)
{
	//TODO: ������Ҫ��nType����ת��, ��װ������ֵ��� m_BPTLib.m_CMAT ���������
	for (int i = 0; i < 2; i++)		// 0: ǰ׺, 1: ��׺
	{
		const KBPT_ClassifiedMAT* pCMAT;
		pCMAT = m_BPTLib.GetCMAT(i, nType);
		if (pCMAT == NULL)
			{ _ASSERT(FALSE); return FALSE; }
		m_GMA_nCount[i] = NUMOFCMA;
		pCMAT->GetAll((int*)m_GMA_naryMA[i], &(m_GMA_nCount[i]));
	}
	return TRUE;
}

/******************************************************************************
	����:	ȷ��������ħ�����Եĵȼ�Ҫ��ͳ��ָ���
	���:	nType: ָ��װ��
			m_GMA_naryMA����: ����ȫ��ħ�����Ե�����ֵ
	����:	�ɹ�ʱ���ط���. m_GMA_naryLevel�����������ħ�����Եĵȼ�Ҫ��
							m_GMA_naryDropRate��������ħ�����Եĳ��ָ���
			ʧ��ʱ������
******************************************************************************/
BOOL KItemGenerator::GMA_GetLevelAndDropRate(int nType)
{
	//TODO: ������Ҫ��nType����ת��, ��װ������ֵ��� m_BPTLib.m_CMAT ���������

	for (int i = 0; i < 2; i++)		// 0: ǰ׺, 1: ��׺
	{
		for (int n = 0; n < m_GMA_nCount[i]; n++)
		{
			const KMAGICATTRIB_TABFILE* pMATF;
			pMATF = GetMARecord(m_GMA_naryMA[i][n]);
			if (pMATF == NULL)
				{ _ASSERT(FALSE); return FALSE; }
			m_GMA_naryLevel[i][n] = pMATF->m_nLevel;
			m_GMA_naryDropRate[i][n] = pMATF->m_DropRate[nType];		// ���Ż�
		}
	}
	return TRUE;
}

/******************************************************************************
	����:	��ȫ�����õ�ħ��������ѡ������������ħ��
	���:	nPos: 0: ѡǰ׺, 1: ѡ��׺
			nLevel: ��ħ�����Եĵȼ�Ҫ��
			nLucky: ����ֵ
			m_GMA_naryMA����: ����ȫ�����õ�ħ�����Ե�����ֵ
	����:	*pINA ������ѡħ���ĺ��Ĳ���
******************************************************************************/
void KItemGenerator::GMA_ChooseMA(int nPos, int nLevel, int nLucky, KItemNormalAttrib* pINA)
{
	_ASSERT(pINA != NULL);

	// ����ֵ
	pINA->nAttribType = 0;
	pINA->nValue[0] = 0;
	pINA->nValue[1] = 0;
	pINA->nValue[2] = 0;

	// ɸѡ��ȫ�����õ�ħ������
	int nDropRate = ::GetRandomNumber(0, 100) - nLucky;
	int nCount = GMA_GetCandidateMA(nPos, nLevel, nDropRate);
	if (nCount > 0)
	{
		int nFinal = ::GetRandomNumber(0, nCount);
		int nMAi = m_GMA_naryCandidateMA[nFinal];
		int nMA = m_GMA_naryMA[nPos][nMAi];
		if (nMA == -1)	// ������ظ���������
		{
			pINA->nAttribType = 0;
			return;
		}
		m_GMA_naryMA[nPos][nMAi] = -1;		// ��Ϊ-1,��ʾ��MA����
		const KMAGICATTRIB_TABFILE* pMATF = GetMARecord(nMA);
		if (NULL != pMATF)
		{
			const KMACP* pMACP = &(pMATF->m_MagicAttrib);
			pINA->nAttribType = pMACP->nPropKind;
			pINA->nValue[0] = ::GetRandomNumber(pMACP->aryRange[0].nMin, pMACP->aryRange[0].nMax);
			pINA->nValue[1] = ::GetRandomNumber(pMACP->aryRange[1].nMin, pMACP->aryRange[1].nMax);
			pINA->nValue[2] = ::GetRandomNumber(pMACP->aryRange[2].nMin, pMACP->aryRange[2].nMax);
		}
	}
}

/******************************************************************************
	����:	��ȡָ����ħ�����Լ�¼
	���:	i: ָ����¼
	����:	�ɹ�ʱ����ָ��ü�¼��ָ��
			ʧ��ʱ����NULL
******************************************************************************/
const KMAGICATTRIB_TABFILE* KItemGenerator::GetMARecord(int i) const
{
	return m_BPTLib.GetMARecord(i);
}

/******************************************************************************
	����:	��ȫ�����õ�ħ��������ѡ������������ħ��
	���:	nPos: 0: ѡǰ׺, 1: ѡ��׺
			nLevel: ��ħ�����Եĵȼ�Ҫ��
			nDropRate: ѡ�����ָ��ʴ��ڴ�ֵ��ħ������
			m_GMA_naryMA����: ����ȫ�����õ�ħ�����Ե�����ֵ
			m_GMA_naryLevel����: ��������ħ�����Եĵȼ�Ҫ��
			m_GMA_naryDropRate����: ��������ħ�����Եĳ��ָ���
	����:	m_GMA_naryCandidateMA�����������������ħ��
	˵��:	�� n = m_GMA_naryMA[nPos][m_GMA_naryCandidateMA[i]]
			�� GetMARecord(n) ����ָ��ħ�����Խṹ��ָ��
******************************************************************************/
int KItemGenerator::GMA_GetCandidateMA(int nPos, int nLevel, int nDropRate)
{
	int nCount = 0;
	for (int i = 0; i < m_GMA_nCount[nPos]; i++)
	{
		if (m_GMA_naryMA[nPos][i] != -1 &&		// -1��ʾ��MA���ù�
			m_GMA_naryLevel[nPos][i] == nLevel &&
			m_GMA_naryDropRate[nPos][i] >= nDropRate)
		{
			m_GMA_naryCandidateMA[nCount++] = i;
		}
	}
	return nCount;
}

BOOL KItemGenerator::GetMedicineCommonAttrib(IN int nDetailType, IN int nLevel, IN OUT KItem* pItem)
{
	if (NULL == pItem)
	{
		KASSERT(FALSE);
		return FALSE;
	}

	const int i = nDetailType * 5 + nLevel - 1;

	const KBASICPROP_MEDICINE* pMed = NULL;
	pMed = m_BPTLib.GetMedicineRecord(i);
	if (NULL == pMed)
	{
		_ASSERT(FALSE);
		return FALSE;
	}
	*pItem = *pMed;
	return TRUE;
}

/******************************************************************************
	���ܣ�	������ɻƽ�װ��װ��
	���:	nCondition: ǰ��������(Emf+Pmf)*Level��������
			pItem: װ����Ŀָ�롣
	����:	�ɹ�ʱ������, ��������� pItem ��ָ�����и���
			ʧ��ʱ���ؼ�
******************************************************************************/
// Add by flying on May.30.2003
// Try to get a "Gold Item" if possible.
BOOL KItemGenerator::GetGoldItemByRandom(IN int nCondition, OUT KItem * pItem)
{
	BOOL bResult = FALSE;
	const KBASICPROP_EQUIPMENT_GOLD* pArray[MAX_ITEM];
	int i = 0;
	int j = 0;
	int nIndex = 0;

	::memset(pArray, 0, sizeof(KBASICPROP_EQUIPMENT_GOLD *) * MAX_ITEM);

	for (i = 0; i < MAX_ITEM; i++)
	{
		const KBASICPROP_EQUIPMENT_GOLD* pTemp = m_BPTLib.GetGoldItemRecord(i);
		if (pTemp == 0)
			break;
		if (pTemp->m_nRarity < nCondition)
			pArray[j] = pTemp;
	}

	if (j == 0)
		goto Exit0;			 
	nIndex = GetRandomNumber(0, j);	
	*pItem = *pArray[nIndex];
Exit0:
	return FALSE;
}

/******************************************************************************
	���ܣ�	����ָ���ƽ�װ��װ��
	���:	nCondition: ǰ��������(Emf+Pmf)*Level��������
			pItem: װ����Ŀָ�롣
	����:	�ɹ�ʱ������, ��������� pItem ��ָ�����и���
			ʧ��ʱ���ؼ�
******************************************************************************/
// Add by flying on May.30.2003
// Try to get a "Gold Item" if possible.
BOOL KItemGenerator::GetGoldItemByIndex(IN int nIndex, OUT KItem * pItem)
{
	BOOL bResult = FALSE;
	int i = 0;
	int j = 0;
	int nCount = 0;

	nCount = m_BPTLib.GetGoldItemNumber();
	const KBASICPROP_EQUIPMENT_GOLD* pTemp = m_BPTLib.GetGoldItemRecord(i);
	if (nIndex >= nCount)
		goto Exit0;
	if (pTemp == NULL)
		goto Exit0;
	*pItem = *pTemp;
	bResult = TRUE;
Exit0:
	return bResult;
}

BOOL KItemGenerator::GetEquipmentCommonAttrib(IN int nDetailType, IN int nParticularType, IN int nLevel, IN int nSeries, IN OUT KItem* pItem)
{
	if (NULL == pItem)
	{
		KASSERT(FALSE);
		return FALSE;
	}

	const int i = nParticularType * 10 + nLevel - 1;// ����ʽ�ɲ߻�����
												// ���SPE 2002��8��31��7:40
												// email: ����������ֵ��
	const KBASICPROP_EQUIPMENT* pEqu = NULL;
	switch(nDetailType)
	{
	case equip_meleeweapon:
		pEqu = m_BPTLib.GetMeleeWeaponRecord(i);
		break;
	case equip_rangeweapon:
		pEqu = m_BPTLib.GetRangeWeaponRecord(i);
		break;
	case equip_armor:
		pEqu = m_BPTLib.GetArmorRecord(i);
		break;
	case equip_helm:
		pEqu = m_BPTLib.GetHelmRecord(i);
		break;
	case equip_boots:
		pEqu = m_BPTLib.GetBootRecord(i);
		break;
	case equip_belt:
		pEqu = m_BPTLib.GetBeltRecord(i);
		break;
	case equip_amulet:
		pEqu = m_BPTLib.GetAmuletRecord(i);
		break;
	case equip_ring:
		pEqu = m_BPTLib.GetRingRecord(i);
		break;
	case equip_cuff:
		pEqu = m_BPTLib.GetCuffRecord(i);
		break;
	case equip_pendant:
		pEqu = m_BPTLib.GetPendantRecord(i);
		break;
	case equip_horse:
		pEqu = m_BPTLib.GetHorseRecord(i);
		break;
	default:
		break;
	}
	if (NULL == pEqu)
		{ _ASSERT(FALSE); return FALSE; }
		// �������˵�ֱ��ԭ��: ֻ��n��װ��, ������ i ��ֵ��[0,n-1]֮��
		// ���3��: nParticularType ����?
		//			nLevel ����?
		//			ԭʼ��tab file������ m_BPTLib.m_BPTEquipment ��
		//			����������������?
	pItem->SetAttrib_CBR(pEqu);
	pItem->SetSeries(nSeries);
	return TRUE;
}

/******************************************************************************
	���ܣ�	�������װ��
	���:	nDetailType: �������, �����������������......
			nParticularType: ��ϸ���
			nSeriesReq: ��������
			nLevel: �ȼ�
			pnaryMALevel: ħ�����Եȼ�����[6]
			nLucky: ����ֵ
	����:	�ɹ�ʱ���ط���, ��������� pItem ��ָ�����и���
			ʧ��ʱ������
******************************************************************************/
BOOL KItemGenerator::Gen_ExistEquipment(IN int nDetailType,
								   IN int nParticularType,
								   IN int nSeriesReq,
								   IN int nLevel,
								   IN const int* pnaryMALevel,
								   IN int nLucky,
								   IN int nVersion,
								   IN OUT KItem* pItem)
{
	_ASSERT(this != NULL);
	_ASSERT(pItem != NULL);

	BOOL bEC = FALSE;

	g_RandomSeed(pItem->m_GeneratorParam.uRandomSeed);
	if (pnaryMALevel)
		memcpy(pItem->m_GeneratorParam.nGeneratorLevel, pnaryMALevel, sizeof(int) * 6);
	int nGameVersion = pItem->m_GeneratorParam.nVersion;

	// ������ڲ���, ȷ��װ���Ļ�������
	const int i = nParticularType * 10 + nLevel - 1;// ����ʽ�ɲ߻�����
												// ���SPE 2002��8��31��7:40
												// email: ����������ֵ��
												// �ȼ����ݴ�1��ʼ������Ҫ������
	const KBASICPROP_EQUIPMENT* pEqu = NULL;
	switch(nDetailType)
	{
	case equip_meleeweapon:
		pEqu = m_BPTLib.GetMeleeWeaponRecord(i);
		break;
	case equip_rangeweapon:
		pEqu = m_BPTLib.GetRangeWeaponRecord(i);
		break;
	case equip_armor:
		pEqu = m_BPTLib.GetArmorRecord(i);
		break;
	case equip_helm:
		pEqu = m_BPTLib.GetHelmRecord(i);
		break;
	case equip_boots:
		pEqu = m_BPTLib.GetBootRecord(i);
		break;
	case equip_belt:
		pEqu = m_BPTLib.GetBeltRecord(i);
		break;
	case equip_amulet:
		pEqu = m_BPTLib.GetAmuletRecord(i);
		break;
	case equip_ring:
		pEqu = m_BPTLib.GetRingRecord(i);
		break;
	case equip_cuff:
		pEqu = m_BPTLib.GetCuffRecord(i);
		break;
	case equip_pendant:
		pEqu = m_BPTLib.GetPendantRecord(i);
		break;
	case equip_horse:
		pEqu = m_BPTLib.GetHorseRecord(i);
		break;
	default:
		break;
	}
	if (NULL == pEqu)
		{ _ASSERT(FALSE); return bEC; }
		// �������˵�ֱ��ԭ��: ֻ��n��װ��, ������ i ��ֵ��[0,n-1]֮��
		// ���3��: nParticularType ����?
		//			nLevel ����?
		//			ԭʼ��tab file������ m_BPTLib.m_BPTEquipment ��
		//			����������������?
	pItem->SetAttrib_CBR(pEqu);
	pItem->SetSeries(nSeriesReq);

	if (NULL == pnaryMALevel)
		return TRUE;
	// ȷ��װ����ħ������
	KItemNormalAttrib	sMA[6];	// ���ߵ�ħ������
	bEC = Gen_MagicAttrib(nDetailType, pnaryMALevel, nSeriesReq, nLucky, sMA, nGameVersion);
	if (bEC)
		pItem->SetAttrib_MA(sMA);

	return bEC;
}

BOOL KItemGenerator::Gen_Quest(IN int nDetailType, IN OUT KItem* pItem)
{
	_ASSERT(this != NULL);
	_ASSERT(pItem != NULL);

	BOOL bEC = FALSE;
	ZeroMemory(&pItem->m_GeneratorParam, sizeof(pItem->m_GeneratorParam));
	
	const KBASICPROP_QUEST* pQuest = NULL;
	pQuest = m_BPTLib.GetQuestRecord(nDetailType);
	if (NULL == pQuest)
	{
		_ASSERT(FALSE);
		return bEC;
	}
	*pItem = *pQuest;
	return TRUE;
}

BOOL KItemGenerator::Gen_TownPortal(IN OUT KItem* pItem)
{
	_ASSERT(this != NULL);
	_ASSERT(pItem != NULL);

	BOOL bEC = FALSE;
	ZeroMemory(&pItem->m_GeneratorParam, sizeof(pItem->m_GeneratorParam));
	
	const KBASICPROP_TOWNPORTAL* pPortal = NULL;
	pPortal = m_BPTLib.GetTownPortalRecord(0);
	if (NULL == pPortal)
	{
		_ASSERT(FALSE);
		return bEC;
	}
	*pItem = *pPortal;
	return TRUE;
}