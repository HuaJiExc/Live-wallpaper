#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#pragma warning(disable : 4996)
LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND MainWindow;
HWND VideoWindow;
//ffplay�����ĺڿ�
HWND ffplayCmd;
//ϵͳ��������
HWND SystemWindow;
//��Ҫ��Ϊ��ֽ�Ĵ���
HWND WallpaperLayerWindow;
LPCWSTR RegValueName(L"Wallpaper");
WCHAR ffplayPath[MAX_PATH];
//CreateProcess�����ڶ���ֵҪ���ַ���ͷΪ�ո�
WCHAR VideoPath[MAX_PATH] = L" ";
/*�洢�ػ�ǰѡ������һ����Ƶ·�����ļ�·��*/
WCHAR LogPath[MAX_PATH];
//��ȡ��Ƶ�ļ�
WCHAR* GetVideoFile();
//�洢�ػ�ǰѡ������һ����Ƶ·�� ��Ϊ�´ο��������ı�ֽ
WCHAR* StorageVideoPath();
//��ȡ�ļ��е���Ƶ·��
WCHAR* ReadFile();
//��ȡ��ȡ��ǰ�ļ���·��
WCHAR* GetFilePath(WCHAR* String, int Size);
//������ֽ
BOOL CreateWallpaper();
//�ҵ�ffplay�Ĳ����ĺڿ�
BOOL FindffplayCmd();
/*�ҵ���Ƶ����*/
BOOL FindVideoWindow();
//ö�ٴ��ڽ��� �ҵ���ֽ��
BOOL WINAPI EnumWindowsProc(HWND hwnd, LPARAM Lparam);
//����������
BOOL WriteRegistry(WCHAR* Program);
/*�޸ı�ֽ*/
BOOL ModifyWallpaper(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
///////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		DWORD PID = 0;
		DWORD VideoWindowPid = GetWindowThreadProcessId(VideoWindow, &PID);
		if (!VideoWindowPid)
		{
			MessageBox(hWnd, L"�Ҳ��� VideoWindowPid\nδ�ɹ��رձ�ֽ", L"����", 0);
			exit(0);
			return 0;
		}
		if (MessageBox(hWnd, L"ȷ���رձ�ֽ?", L"�ر�", MB_OKCANCEL))
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
			TerminateProcess(hProcess, 0);
			//����WorkerW��������ʾˢ�µ��������������Ƶ����
			ShowWindow(WallpaperLayerWindow, 0);
			ShowWindow(WallpaperLayerWindow, 10);
			exit(0);
			return 0;
		}
		return 0;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
WCHAR* GetVideoFile()
{
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);	//�ṹ���С
	ofn.hwndOwner = NULL;					//���ര��
	ofn.lpstrFilter = TEXT("MP4|*.mp4");	//�ļ�����
	ofn.nFilterIndex = 1;	//����������
	WCHAR FileName[MAX_PATH] = {};	//���������ļ���
	ofn.lpstrFile = FileName;	//���շ��ص��ļ���
	ofn.nMaxFile = sizeof(FileName);	//����������
	ofn.lpstrInitialDir = NULL;			//��ʼĿ¼
	ofn.lpstrTitle = TEXT("ѡ����Ƶ�ļ�"); //���ڱ���
	//���ļ��Ի���
	if (!GetOpenFileName(&ofn))
	{
		//��δѡ���ļ� ����ֹ����
		exit(0);
		return 0;
	}
	wcscat(VideoPath, FileName);
	//�õ���Ƶ�ļ�ָ��
	wcscat(VideoPath, L" -noborder -x 1920 -y 1080 -loop 0\0");
	return VideoPath;
}
WCHAR* StorageVideoPath()
{
	wfstream Wios;
	//��GBK���봦���ַ�
	Wios.imbue(locale("chs"));
	GetVideoFile();
	GetModuleFileName(NULL, LogPath, MAX_PATH);
	GetFilePath(LogPath, sizeof(LogPath));
	//���ļ��洢�ڵ�ǰ�ļ�·���µ�bin\log
	wcscat(LogPath, L"bin\\Log\0");
	//����ȡ������Ƶ�ļ�·���ַ���д�뵽�ļ�
	Wios.open(LogPath, ios::out);
	Wios << VideoPath;
	Wios.close();
	return VideoPath;
}
WCHAR* ReadFile()
{
	wfstream Wios;
	//��GBK���봦���ַ�
	Wios.imbue(locale("chs"));
	GetModuleFileName(NULL, LogPath, MAX_PATH);
	GetFilePath(LogPath, sizeof(LogPath));
	//�� ��ǰ�ļ���\bin\log
	wcscat(LogPath, L"bin\\Log\0");
	Wios.open(LogPath, ios::in);
	wstring wstr;
	WCHAR VideoFile[MAX_PATH];
	while (getline(Wios, wstr))
	{
		for (int i = wstr.size(); i > -1; i--)
		{
			VideoFile[i] = wstr[i];
		}
	}
	return VideoFile;
}
WCHAR* GetFilePath(WCHAR* String, int Size)
{
	//����Сת��Ϊ�м���ֵ
	Size /= 2;
	//���ַ���β����ʼ����
	for (; towlower(String[Size]) != 92; Size--)
	{
		//����ǰ��ִ���ļ����ַ����
		String[Size] = NULL;
	}
	//���շ��ص�ǰ��ִ�г���ĸ��ļ���·��
	return String;
}
BOOL CreateWallpaper()
{
	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	LPWSTR lPwstr = ReadFile();
	//��ffplay
	if (CreateProcess(ffplayPath, ReadFile(), 0, 0, 0, 0, 0, 0, &si, &pi))
	{
		//�ҵ�ϵͳ��������
		SystemWindow = FindWindow(L"Progman", 0);
		//��ϵͳ�������ڷ���0x52C��Ϣ��ʹ�����Լ�ǰ����ѳ���������
		SendMessageTimeout(SystemWindow, 0x52C, 0, 0, 0, 100, 0);
		//���²ο�BiliBili UP��ż���е�С�Ժ�����Ƶ ��Ƶ��:BV1HZ4y1978a

		//       |
		// SystemWindow << 0x52C��Ϣ
		//      ��                     
		//       |                 |                    /|*      ��"*"�������ݷ�ʽ��
		// SystemWindow     ���ѵĵ�һ������    ���ѵĵڶ�������

		//���涨���SystemWindow(ϵͳ��������)����ײ�

		/*���ѵĵ�һ������: |(����ע�ͼ�Ʊ�ֽ��)				
		������ϵͳ���ڵı���                                     
		Ҫ����Ƶ��������һ��,Ҳ���ǽ���Ƶ��������Ϊ������ڵ��Ӵ���*/

		/*���ѵĵڶ�������: /|*											
		������������ϲ��,͸���ģ����������ݷ�ʽ����һ�㴰��
		��������һ������Ϊ"SHELLDLL_DefView"���Ӵ���*/

		//ö�ٳ���ֽ�㴰��
		EnumWindows(EnumWindowsProc, 0);
		//�ҵ�ffplay�����ĺڿ򣬲�����
		while (!ShowWindow(ffplayCmd, SW_HIDE))
		{
			FindffplayCmd();
		}
		/*�ҵ���Ƶ���ڣ���ʱ����
		��ֹ���ŵ���Ƶ��û�б�����Ϊ��ֽ�����ס����*/
		while (!ShowWindow(VideoWindow, 0))
		{
			FindVideoWindow();
		}
		//����Ƶ��������Ϊ��ֽ����Ӵ���
		SetParent(VideoWindow, WallpaperLayerWindow);
		//���û���ҵ���Ƶ����
		if (VideoWindow == NULL)
		{
			//������ֹ����
			MessageBox(NULL, wcscat(VideoPath,
				L"\nδ�ҵ�VideoWindow����\n��Ƶ�Ƿ�ɹ�����?\n�뷴����������!"), L" ����", 0);
			exit(0);
			return false;
		}
		//����Ƶ���ڶ��� ����޶ȵĸ�������ʾ������������
		SetWindowPos(VideoWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE);
		//��ʾ��Ƶ����
		ShowWindow(VideoWindow, 1);
		return true;
	}
	else
	{
		//����ʧ���򱨴� ����ֹ����
		MessageBox(NULL, L"��ʧ��...\nδ��ffplay.exe", L"����", NULL);
		exit(0);
		return false;
	}
	return false;
}
BOOL FindffplayCmd()
{
	ffplayCmd = FindWindow(L"CASCADIA_HOSTING_WINDOW_CLASS", 0);
	if (ffplayCmd == 0)
	{
		ffplayCmd = FindWindow(L"CASCADIA_HOSTING_WINDOW_CLASS", 0);
		return true;
	}
	return false;
}
BOOL FindVideoWindow()
{
	//���غڿ�
	ShowWindow(ffplayCmd, SW_HIDE);
	VideoWindow = FindWindow(L"SDL_app", 0);
	if (VideoWindow == 0)
	{
		VideoWindow = FindWindow(L"SDL_app", 0);
		return true;
	}
	return false;
}
BOOL WINAPI EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM Lparam)
{
	//�ҵ�����ΪSHELLDLL_DefView���Ӵ���
	HWND SDWindow = FindWindowEx(hwnd, 0, L"SHELLDLL_DefView", 0);
	//���ҵ�
	if (SDWindow != 0) 
	{
		//����һ�����ھ��Ǳ�ֽ��
		WallpaperLayerWindow = FindWindowEx(0, hwnd, L"WorkerW", 0);
		return FALSE;
	}
	return TRUE;
}
BOOL WriteRegistry(WCHAR *Program)
{
	HKEY Key = NULL;
	//��HKEY_CURRENT_USERע�����
	LSTATUS RC = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &Key);
	//���ע���HKEY_CURRENT_USER���ڵ�"Wallpaper"ֵ�����
	if (RegGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		RegValueName, RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		//������ ������д��һ����ֹע���ֵ��Ϣ����
		RC = RegSetValueEx(Key, RegValueName, 0, REG_SZ, (CONST BYTE*)Program, MAX_PATH);
		//�رմ˼�
		RegCloseKey(Key);
		return true;
	}
	else
	{
		RC = RegSetValueEx(Key, RegValueName, 0, REG_SZ, (CONST BYTE*)Program,MAX_PATH);
		if (RC)
		{
			MessageBox(MainWindow, L"��������������ʧ�ܣ�\n������ע���ֵδ�ɹ�д��", L"����", NULL);
			return false;
		}
		else
		{
			RegCloseKey(Key);
			MessageBox(NULL, L"���óɹ�!", L"����������", NULL);
			return true;
		}
	}
}
BOOL ModifyWallpaper(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//�ȼ���Ƿ���ڵ�ǰ���򴴽��Ĵ�������鵱ǰ�����Ƿ�������
	HWND MainWindow = FindWindow(L"VideoWallpaperSwitch",0);
	//ö�ٳ�WallpaperLayer����
	EnumWindows(EnumWindowsProc, 0);
	//�ҵ���Ƶ����
	HWND VideoWindow = FindWindowEx(WallpaperLayerWindow,NULL,L"SDL_app",0);
	if (!MainWindow)
	{
		//���û������
		DWORD VideoProgramPID = 0;
		GetWindowThreadProcessId(VideoWindow, &VideoProgramPID);
		HANDLE VideoProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, VideoProgramPID);
		//�ر��ϴ�û�йرյ���Ƶ���ڽ���
		TerminateProcess(VideoProcess, 0);
		return false;
	}
	else
	{
		//��ȡ��Ƶ����PID
		DWORD VideoProgramPID = 0;
		GetWindowThreadProcessId(VideoWindow, &VideoProgramPID);
		//��ȡ��ǰ�����PID
		DWORD MainProgramPid = 0;
		GetWindowThreadProcessId(MainWindow, &MainProgramPid);
		if (MessageBox(NULL, L"�Ƿ�Ҫ����ѡ���ֽ", L"����ѡ���ֽ", MB_OKCANCEL))
		{
			//���ļ��ж�ȡ�ϴδ򿪵���Ƶ�ļ�·�����ַ���
			StorageVideoPath();
			//��ȡffplay·��
			GetModuleFileName(NULL, ffplayPath, MAX_PATH);
			GetFilePath(ffplayPath, sizeof(ffplayPath));
			wcscat(ffplayPath, L"bin\\ffplay.exe");
			//��ֹ�ϴ�������ǰ����Ľ���
			HANDLE VideoProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, VideoProgramPID);
			HANDLE MainProgramPocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, MainProgramPid);
			TerminateProcess(VideoProcess, 0);
			TerminateProcess(MainProgramPocess, 0);
			if (CreateWallpaper())
			{
				//�����ֽ�����ɹ��򴴽�һ������
				WNDCLASS WinClass = { 0 };
				WinClass.lpfnWndProc = WindowProc;
				WinClass.lpszClassName = L"VideoWallpaperSwitch";
				WinClass.hInstance = hInstance;
				RegisterClass(&WinClass);
				MainWindow = CreateWindowEx(0, L"VideoWallpaperSwitch", L"VWS", WS_EX_LAYERED
					, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					NULL, NULL, hInstance, NULL);
				MSG Msg;
				while (GetMessage(&Msg, MainWindow, 0, 0))
				{
					//ʹ�����ޱ߿�
					SetWindowLong(MainWindow, GWL_STYLE, GetWindowLong(MainWindow, GWL_STYLE) & ~(WS_CAPTION | WS_SIZEBOX));
					//ʹ����͸��
					SetLayeredWindowAttributes(MainWindow, 0, 0, LWA_COLORKEY);
					//��������С��
					ShowWindow(MainWindow, SW_SHOWMINIMIZED);
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
				return true;
			}
			else
			{
				MessageBox(NULL, L"���´�����ֽʧ��,\n������ffplay��δ�ɹ���,\n�����ִ���ļ����ļ�����bin\\ffplay.exe�Ƿ����,�����������ߣ�",
					L"����", NULL);
				exit(0);
			}
		}
		return true;
	}
}