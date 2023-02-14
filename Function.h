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
//ffplay产生的黑框
HWND ffplayCmd;
//系统背景窗口
HWND SystemWindow;
//将要作为壁纸的窗口
HWND WallpaperLayerWindow;
LPCWSTR RegValueName(L"Wallpaper");
WCHAR ffplayPath[MAX_PATH];
//CreateProcess函数第二个值要求字符串头为空格
WCHAR VideoPath[MAX_PATH] = L" ";
/*存储关机前选择的最后一次视频路径的文件路径*/
WCHAR LogPath[MAX_PATH];
//获取视频文件
WCHAR* GetVideoFile();
//存储关机前选择的最后一次视频路径 作为下次开机启动的壁纸
WCHAR* StorageVideoPath();
//读取文件中的视频路径
WCHAR* ReadFile();
//获取获取当前文件夹路径
WCHAR* GetFilePath(WCHAR* String, int Size);
//创建壁纸
BOOL CreateWallpaper();
//找到ffplay的产生的黑框
BOOL FindffplayCmd();
/*找到视频窗口*/
BOOL FindVideoWindow();
//枚举窗口进程 找到壁纸层
BOOL WINAPI EnumWindowsProc(HWND hwnd, LPARAM Lparam);
//开机自启动
BOOL WriteRegistry(WCHAR* Program);
/*修改壁纸*/
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
			MessageBox(hWnd, L"找不到 VideoWindowPid\n未成功关闭壁纸", L"错误", 0);
			exit(0);
			return 0;
		}
		if (MessageBox(hWnd, L"确定关闭壁纸?", L"关闭", MB_OKCANCEL))
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
			TerminateProcess(hProcess, 0);
			//隐藏WorkerW窗口再显示刷新掉残留在上面的视频背景
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
	ofn.lStructSize = sizeof(OPENFILENAME);	//结构体大小
	ofn.hwndOwner = NULL;					//父类窗口
	ofn.lpstrFilter = TEXT("MP4|*.mp4");	//文件过滤
	ofn.nFilterIndex = 1;	//过滤器索引
	WCHAR FileName[MAX_PATH] = {};	//用来接收文件名
	ofn.lpstrFile = FileName;	//接收返回的文件名
	ofn.nMaxFile = sizeof(FileName);	//缓冲区长度
	ofn.lpstrInitialDir = NULL;			//初始目录
	ofn.lpstrTitle = TEXT("选择视频文件"); //窗口标题
	//打开文件对话框
	if (!GetOpenFileName(&ofn))
	{
		//若未选择文件 则终止程序
		exit(0);
		return 0;
	}
	wcscat(VideoPath, FileName);
	//得到视频文件指令
	wcscat(VideoPath, L" -noborder -x 1920 -y 1080 -loop 0\0");
	return VideoPath;
}
WCHAR* StorageVideoPath()
{
	wfstream Wios;
	//以GBK编码处理字符
	Wios.imbue(locale("chs"));
	GetVideoFile();
	GetModuleFileName(NULL, LogPath, MAX_PATH);
	GetFilePath(LogPath, sizeof(LogPath));
	//将文件存储在当前文件路径下的bin\log
	wcscat(LogPath, L"bin\\Log\0");
	//将获取到的视频文件路径字符串写入到文件
	Wios.open(LogPath, ios::out);
	Wios << VideoPath;
	Wios.close();
	return VideoPath;
}
WCHAR* ReadFile()
{
	wfstream Wios;
	//以GBK编码处理字符
	Wios.imbue(locale("chs"));
	GetModuleFileName(NULL, LogPath, MAX_PATH);
	GetFilePath(LogPath, sizeof(LogPath));
	//打开 当前文件夹\bin\log
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
	//将大小转换为有几个值
	Size /= 2;
	//从字符串尾部开始访问
	for (; towlower(String[Size]) != 92; Size--)
	{
		//将当前可执行文件的字符清除
		String[Size] = NULL;
	}
	//最终返回当前可执行程序的父文件夹路径
	return String;
}
BOOL CreateWallpaper()
{
	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	LPWSTR lPwstr = ReadFile();
	//打开ffplay
	if (CreateProcess(ffplayPath, ReadFile(), 0, 0, 0, 0, 0, 0, &si, &pi))
	{
		//找到系统背景窗口
		SystemWindow = FindWindow(L"Progman", 0);
		//向系统背景窗口发送0x52C消息，使它在自己前面分裂出两个窗口
		SendMessageTimeout(SystemWindow, 0x52C, 0, 0, 0, 100, 0);
		//以下参考BiliBili UP主偶尔有点小迷糊的视频 视频号:BV1HZ4y1978a

		//       |
		// SystemWindow << 0x52C消息
		//      ↓                     
		//       |                 |                    /|*      （"*"是桌面快捷方式）
		// SystemWindow     分裂的第一个窗口    分裂的第二个窗口

		//上面定义的SystemWindow(系统背景窗口)在最底层

		/*分裂的第一个窗口: |(以下注释简称壁纸层)				
		复制了系统窗口的背景                                     
		要将视频放置在这一层,也就是将视频窗口设置为这个窗口的子窗口*/

		/*分裂的第二个窗口: /|*											
		这个窗口是最上层的,透明的，但是桌面快捷方式在这一层窗口
		而且它有一个类名为"SHELLDLL_DefView"的子窗口*/

		//枚举出壁纸层窗口
		EnumWindows(EnumWindowsProc, 0);
		//找到ffplay产生的黑框，并隐藏
		while (!ShowWindow(ffplayCmd, SW_HIDE))
		{
			FindffplayCmd();
		}
		/*找到视频窗口，暂时隐藏
		防止播放的视频还没有被设置为壁纸层而挡住桌面*/
		while (!ShowWindow(VideoWindow, 0))
		{
			FindVideoWindow();
		}
		//将视频窗口设置为壁纸层的子窗口
		SetParent(VideoWindow, WallpaperLayerWindow);
		//如果没有找到视频窗口
		if (VideoWindow == NULL)
		{
			//报错并终止程序
			MessageBox(NULL, wcscat(VideoPath,
				L"\n未找到VideoWindow窗口\n视频是否成功播放?\n请反馈给开发者!"), L" 错误", 0);
			exit(0);
			return false;
		}
		//将视频窗口对齐 最大限度的覆盖在显示器的所有区域
		SetWindowPos(VideoWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE);
		//显示视频窗口
		ShowWindow(VideoWindow, 1);
		return true;
	}
	else
	{
		//若打开失败则报错 并终止程序
		MessageBox(NULL, L"打开失败...\n未打开ffplay.exe", L"错误", NULL);
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
	//隐藏黑框
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
	//找到类名为SHELLDLL_DefView的子窗口
	HWND SDWindow = FindWindowEx(hwnd, 0, L"SHELLDLL_DefView", 0);
	//若找到
	if (SDWindow != 0) 
	{
		//则下一个窗口就是壁纸层
		WallpaperLayerWindow = FindWindowEx(0, hwnd, L"WorkerW", 0);
		return FALSE;
	}
	return TRUE;
}
BOOL WriteRegistry(WCHAR *Program)
{
	HKEY Key = NULL;
	//打开HKEY_CURRENT_USER注册表项
	LSTATUS RC = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &Key);
	//检查注册表HKEY_CURRENT_USER项内的"Wallpaper"值否存在
	if (RegGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		RegValueName, RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		//若存在 则重新写入一个防止注册表值信息错误
		RC = RegSetValueEx(Key, RegValueName, 0, REG_SZ, (CONST BYTE*)Program, MAX_PATH);
		//关闭此键
		RegCloseKey(Key);
		return true;
	}
	else
	{
		RC = RegSetValueEx(Key, RegValueName, 0, REG_SZ, (CONST BYTE*)Program,MAX_PATH);
		if (RC)
		{
			MessageBox(MainWindow, L"开机自启动设置失败，\n可能是注册表值未成功写入", L"错误", NULL);
			return false;
		}
		else
		{
			RegCloseKey(Key);
			MessageBox(NULL, L"设置成功!", L"开机自启动", NULL);
			return true;
		}
	}
}
BOOL ModifyWallpaper(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//先检查是否存在当前程序创建的窗口来检查当前程序是否在运行
	HWND MainWindow = FindWindow(L"VideoWallpaperSwitch",0);
	//枚举出WallpaperLayer窗口
	EnumWindows(EnumWindowsProc, 0);
	//找到视频窗口
	HWND VideoWindow = FindWindowEx(WallpaperLayerWindow,NULL,L"SDL_app",0);
	if (!MainWindow)
	{
		//如果没有运行
		DWORD VideoProgramPID = 0;
		GetWindowThreadProcessId(VideoWindow, &VideoProgramPID);
		HANDLE VideoProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, VideoProgramPID);
		//关闭上次没有关闭的视频窗口进程
		TerminateProcess(VideoProcess, 0);
		return false;
	}
	else
	{
		//获取视频窗口PID
		DWORD VideoProgramPID = 0;
		GetWindowThreadProcessId(VideoWindow, &VideoProgramPID);
		//获取当前程序的PID
		DWORD MainProgramPid = 0;
		GetWindowThreadProcessId(MainWindow, &MainProgramPid);
		if (MessageBox(NULL, L"是否要重新选择壁纸", L"重新选择壁纸", MB_OKCANCEL))
		{
			//从文件中读取上次打开的视频文件路径的字符串
			StorageVideoPath();
			//获取ffplay路径
			GetModuleFileName(NULL, ffplayPath, MAX_PATH);
			GetFilePath(ffplayPath, sizeof(ffplayPath));
			wcscat(ffplayPath, L"bin\\ffplay.exe");
			//终止上次启动当前程序的进程
			HANDLE VideoProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, VideoProgramPID);
			HANDLE MainProgramPocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, MainProgramPid);
			TerminateProcess(VideoProcess, 0);
			TerminateProcess(MainProgramPocess, 0);
			if (CreateWallpaper())
			{
				//如果壁纸创建成功则创建一个窗口
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
					//使窗口无边框
					SetWindowLong(MainWindow, GWL_STYLE, GetWindowLong(MainWindow, GWL_STYLE) & ~(WS_CAPTION | WS_SIZEBOX));
					//使窗口透明
					SetLayeredWindowAttributes(MainWindow, 0, 0, LWA_COLORKEY);
					//将窗口最小化
					ShowWindow(MainWindow, SW_SHOWMINIMIZED);
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
				return true;
			}
			else
			{
				MessageBox(NULL, L"重新创建壁纸失败,\n可能是ffplay并未成功打开,\n请检查可执行文件的文件夹下bin\\ffplay.exe是否存在,或反馈给开发者！",
					L"错误", NULL);
				exit(0);
			}
		}
		return true;
	}
}