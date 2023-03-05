// D3D_Shell.h
// ���� Direct3D ����������Ϣ.
// ����Ⱦģ����CD3D_Shell����ֻ��һ��.

#include "precompile.h"
#include "d3d_shell.h"
#include <algorithm>
#include <tchar.h>

CD3D_Shell g_D3DShell;						// The global D3D Shell...
D3DFORMAT g_PixelFormat[3] = {D3DFMT_X1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X8R8G8B8};

// Create the Sucka...
bool CD3D_Shell::Create()
{
	FreeAll();								// Make sure everything is all clean before we start...

	// ���� D3D ����(ͨ�������Բ�ѯ�ʹ���D3D�豸)...
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D) return false;

	// ȡ��������ʾģʽ...
	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_DesktopFormat))) { FreeAll(); return false; }

	// �����豸�б� (������, �豸, ��ʾģʽ)...
	if (!BuildDeviceList()) { FreeAll(); return false; }
	
	return true;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Shell::Reset()
{
	m_pD3D = NULL;

	m_AdapterList.clear();
}

// Frees all the member vars and resets afterwards...
void CD3D_Shell::FreeAll()
{
	if (m_pD3D) {
		uint32 iRefCnt = m_pD3D->Release(); } // assert(iRefCnt==0);

	Reset();
}

// �����豸�б� - �����ǻ�������:
//	1. ѭ������ϵͳ�е����������� (ͨ��ֻ��һ��),
//	2. �оٴ��豸��������ʾģʽ�����ظ�ʽ.
//	3. �������豸�����������ṹ.
//	4. ѡ��һ��ȱʡ���������豸����ʾģʽ...
bool CD3D_Shell::BuildDeviceList()
{
	if (!m_pD3D) return false;
	m_AdapterList.clear();				// Clear the Adapter List First...

	// Enumerate all adapters on this machine.
	for (UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); ++iAdapter)
	{
		D3DAdapterInfo AdapterInfo; // Fill in adapter info
		AdapterInfo.iAdapterNum = iAdapter;
		m_pD3D->GetAdapterIdentifier(iAdapter, 0, &AdapterInfo.AdapterID);
		AdapterInfo.iNumDevices = 0;
		AdapterInfo.iCurrentDevice = 0;

		// Enumerate all display modes and formats on this adapter...
		vector<D3DModeInfo> modes;
		vector<D3DFORMAT> formats;
		for (int i = 0; i < 3; i++)
		{
			uint32_t iNumAdapterModes = m_pD3D->GetAdapterModeCount(iAdapter, g_PixelFormat[i]);

			D3DDISPLAYMODE DesktopMode;
			m_pD3D->GetAdapterDisplayMode(iAdapter, &DesktopMode);
			formats.push_back(DesktopMode.Format);

			// Enumerate all display modes for this adapter and format
			for (uint32_t iMode = 0; iMode < iNumAdapterModes; iMode++)
			{
				D3DDISPLAYMODE d3dDisplayMode;
				m_pD3D->EnumAdapterModes(iAdapter, g_PixelFormat[i], iMode, &d3dDisplayMode);
				D3DModeInfo DisplayMode;
				DisplayMode.Width = d3dDisplayMode.Width;
				DisplayMode.Height = d3dDisplayMode.Height;
				DisplayMode.Format = d3dDisplayMode.Format;

				// Filter out low-resolution modes
				if (DisplayMode.Width < 640 || DisplayMode.Height < 480)
				{
					continue;
				}

				// Check if mode is already in the list
				bool bModeAlreadyExists = false;
				for (uint32_t m = 0; m < modes.size(); ++m)
				{
					if (modes[m].Width == DisplayMode.Width && modes[m].Height == DisplayMode.Height && modes[m].Format == DisplayMode.Format)
					{
						bModeAlreadyExists = true;
						break;
					}
				}

				// Add mode to the list if it does not exist
				if (!bModeAlreadyExists)
				{
					modes.push_back(DisplayMode);
				}
			}
		}

		// Sort the display modes by resolution
		sort(modes.begin(), modes.end());

		// Add device types to the list
		// Add device types to the list
		vector<D3DDEVTYPE> DeviceTypes = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };
		vector<const TCHAR*> strDeviceDescs = { _T("HAL"), _T("REF") };

		for (uint32_t iDevice = 0; iDevice < DeviceTypes.size(); ++iDevice)
		{
			D3DDeviceInfo Device;
			Device.DeviceType = DeviceTypes[iDevice];
			m_pD3D->GetDeviceCaps(iAdapter, DeviceTypes[iDevice], &Device.d3dCaps);
			TCHAR* strDesc = const_cast<TCHAR*>(strDeviceDescs[iDevice]);
			Device.strDesc = strDesc;
			Device.iCurrentMode = 0;
			Device.bCanDoWindowed = FALSE;
			Device.bWindowed = FALSE;
			Device.bStereo = FALSE;
			Device.MultiSampleType = D3DMULTISAMPLE_NONE;

			// Enumerate all display modes and formats for this device...
			vector<bool> bConfirmedFormats;
			vector<bool> bCanDoHWTnL;
			for (DWORD f = 0; f < formats.size(); ++f)
			{
				bool bConfirmed = false; bool bHWTnL = false;

				// Check if this device can support this format
				if (FAILED(m_pD3D->CheckDeviceType(iAdapter, Device.DeviceType, formats[f], formats[f], FALSE)))
				{
					bConfirmed = false;
				}
				else
				{
					bConfirmed = true;
					// Confirm the device for HW vertex processing
					if (Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
						bHWTnL = true;
					else
						bHWTnL = false;
				}

				bConfirmedFormats.push_back(bConfirmed);
				bCanDoHWTnL.push_back(bHWTnL);
			}

			// Add display modes and formats to the device list
			for (uint32_t m = 0; m < modes.size(); ++m)
			{
				for (uint32_t f = 0; f < formats.size(); ++f)
				{
					if (modes[m].Format == formats[f])
					{
						if (bConfirmedFormats[f])
						{
							modes[m].bHWTnL = bCanDoHWTnL[f];
							Device.Modes.push_back(modes[m]);
						}
					}
				}
			}

			// Choose a default mode of 800x600
			for (uint32_t m = 0; m < Device.Modes.size(); ++m)
			{
				if (Device.Modes[m].Width == 800 && Device.Modes[m].Height == 600)
				{
					Device.iCurrentMode = m;
					if (Device.Modes[m].Format == D3DFMT_R5G6B5 || Device.Modes[m].Format == D3DFMT_X1R5G5B5 || Device.Modes[m].Format == D3DFMT_A1R5G5B5)
						break;
				}
			}

			// Check if the device can support windowed mode (using the first format)
			if (bConfirmedFormats[0])
			{
				Device.bCanDoWindowed = TRUE;
				Device.bWindowed = TRUE;
			}

			// Add the device to the adapter list
			AdapterInfo.Devices.push_back(Device);
		}

		// Add the adapter to the adapter list
		m_AdapterList.push_back(AdapterInfo);
}

    // Return an error if no compatible devices were found
    if (!m_AdapterList.size()) return false;

    return true;
}

// �ɳ�ʼ���������ã�ѡ��ʹ����豸 (g_Device)...
//	Pick Device Named szDevName if there's one of this name, otherwise, will pick what it thinks is best...
D3DDeviceInfo* CD3D_Shell::PickDefaultDev(D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			if (g_bRefRast && itDevice->DeviceType != D3DDEVTYPE_REF) continue;
			if (!g_bRefRast && itDevice->DeviceType != D3DDEVTYPE_HAL) continue;
			if (g_bRunWindowed && !itDevice->bWindowed) continue;

			*pAdapterInfo = &(*itAdapter); 
            return &(*itDevice);
		}
	} 
	return NULL;
}

D3DModeInfo* CD3D_Shell::PickDefaultMode(D3DDeviceInfo* pDeviceInfo,uint32 iBitDepth)
{
	for (vector<D3DModeInfo>::iterator itMode = pDeviceInfo->Modes.begin(); itMode != pDeviceInfo->Modes.end(); ++itMode)
	{
		if (itMode->Width  != g_nScreenWidth)  continue;
		if (itMode->Height != g_nScreenHeight) continue;
		if (g_bRunWindowed && itMode->Format != m_DesktopFormat.Format) continue; 
		if (!g_bRunWindowed) {
			switch (iBitDepth) {
			case 32 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 24 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 16 : if (itMode->Format != D3DFMT_R5G6B5   && itMode->Format != D3DFMT_X1R5G5B5) continue; break; } }

		return &(*itMode);
	}
	return NULL;
}

// �������е�������/�豸�������Ǵ�ӡ����
void CD3D_Shell::ListDevices()
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			g_DebugLog("Device: %s", itDevice->strDesc);
		}
	}
}

D3DDeviceInfo* CD3D_Shell::FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			if (strcmp(strDesc,itDevice->strDesc)==0)
			{
				*pAdapterInfo = &(*itAdapter); return &(*itDevice); 
			}
		}
	}
	return NULL;
}

D3DAdapterInfo*	CD3D_Shell::GetAdapterInfo(unsigned int uAdapterID)
{
	if (uAdapterID < m_AdapterList.size())
		return &m_AdapterList[uAdapterID];
	return NULL;
}

D3DDeviceInfo*	CD3D_Shell::GetDeviceInfo(unsigned int uAdapterID, unsigned int uDeviceID)
{
	D3DAdapterInfo* pAdapter = GetAdapterInfo(uAdapterID);
	if (pAdapter)
	{
		if (uDeviceID < pAdapter->Devices.size())
			return &pAdapter->Devices[uDeviceID];
	}
	return NULL;
}

D3DModeInfo* CD3D_Shell::GetModeInfo(unsigned int uAdapterID,unsigned int uDeviceID,unsigned int uModeID)
{
	D3DDeviceInfo* pDevice = GetDeviceInfo(uAdapterID, uDeviceID);
	if (pDevice)
	{
		if (uModeID < pDevice->Modes.size())
			return &pDevice->Modes[uModeID];
	}
	return NULL;
}