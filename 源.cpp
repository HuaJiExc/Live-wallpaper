#include "Function.h"
using namespace std;
#pragma warning(disable : 4996)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//检查当前程序是否在运行
	ModifyWallpaper(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	//设置开机自启动   //获取当前可执行程序路径
	GetModuleFileName(NULL, ffplayPath, MAX_PATH);
	if (!WriteRegistry(ffplayPath))
	{
		MessageBox(NULL, L"开机自启动设置失败\n写入注册表失败", L"错误", 0);
	}
	//获取ffplay播放器路径
	GetFilePath(ffplayPath, sizeof(ffplayPath));
	wcscat(ffplayPath, L"bin\\ffplay.exe\0");
	if (!CreateWallpaper())
	{
		//若返回false
		//则报错 并终止程序
		MessageBox(NULL, L"壁纸创建失败", L"错误", NULL);
		exit(0);
		return 0;
	}
	else
	{
		//若返回true
		//则 创建窗口
		WNDCLASS WinClass = { 0 };
		WinClass.lpfnWndProc = WindowProc;
		WinClass.lpszClassName = L"VideoWallpaperSwitch";
		WinClass.hInstance = hInstance;
		RegisterClass(&WinClass);
		MainWindow = CreateWindowEx(0, L"VideoWallpaperSwitch", L"VWS", WS_EX_LAYERED
			, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);
		MSG Msg;
		//消息循环
		while (GetMessage(&Msg, MainWindow, 0, 0))
		{
			//使窗口无边框
			SetWindowLong(MainWindow, GWL_STYLE, GetWindowLong(MainWindow, GWL_STYLE) & ~(WS_CAPTION | WS_SIZEBOX));
			//使窗口透明
			SetLayeredWindowAttributes(MainWindow, 0, 0, LWA_COLORKEY);
			//将窗口最小化
			ShowWindow(MainWindow, SW_SHOWMINIMIZED);
			//获取窗口消息
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	return 0;
}