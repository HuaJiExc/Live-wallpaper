#include "Function.h"
using namespace std;
#pragma warning(disable : 4996)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//��鵱ǰ�����Ƿ�������
	ModifyWallpaper(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	//���ÿ���������   //��ȡ��ǰ��ִ�г���·��
	GetModuleFileName(NULL, ffplayPath, MAX_PATH);
	if (!WriteRegistry(ffplayPath))
	{
		MessageBox(NULL, L"��������������ʧ��\nд��ע���ʧ��", L"����", 0);
	}
	//��ȡffplay������·��
	GetFilePath(ffplayPath, sizeof(ffplayPath));
	wcscat(ffplayPath, L"bin\\ffplay.exe\0");
	if (!CreateWallpaper())
	{
		//������false
		//�򱨴� ����ֹ����
		MessageBox(NULL, L"��ֽ����ʧ��", L"����", NULL);
		exit(0);
		return 0;
	}
	else
	{
		//������true
		//�� ��������
		WNDCLASS WinClass = { 0 };
		WinClass.lpfnWndProc = WindowProc;
		WinClass.lpszClassName = L"VideoWallpaperSwitch";
		WinClass.hInstance = hInstance;
		RegisterClass(&WinClass);
		MainWindow = CreateWindowEx(0, L"VideoWallpaperSwitch", L"VWS", WS_EX_LAYERED
			, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);
		MSG Msg;
		//��Ϣѭ��
		while (GetMessage(&Msg, MainWindow, 0, 0))
		{
			//ʹ�����ޱ߿�
			SetWindowLong(MainWindow, GWL_STYLE, GetWindowLong(MainWindow, GWL_STYLE) & ~(WS_CAPTION | WS_SIZEBOX));
			//ʹ����͸��
			SetLayeredWindowAttributes(MainWindow, 0, 0, LWA_COLORKEY);
			//��������С��
			ShowWindow(MainWindow, SW_SHOWMINIMIZED);
			//��ȡ������Ϣ
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	return 0;
}