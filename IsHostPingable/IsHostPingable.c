#include <Windows.h>
#include <stdio.h>
#include <CommCtrl.h>
#include "resource.h"
#define IDT_TIMER1 20190322

WCHAR sHost[MAX_PATH];
WCHAR sHost_Prev[MAX_PATH] = L"ISHOSTPINGABLE";
WCHAR sCMD[MAX_PATH];

HWND hWnd;
HWND hEditHost;
HWND hIMG;
HWND hProgressBar;

HANDLE hPingable;
HANDLE hPingable_Not;
HANDLE hProcessing;

STARTUPINFO si;
PROCESS_INFORMATION pi;

HMODULE hModule;

DWORD nExitCode;

DWORD WINAPI ThreadGUIProc(PVOID pM)
{
	SendMessage(hIMG, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hProcessing);

	SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
	SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM)1, 0);

	// USER_TIMER_MINIMUM 0x0A cannot be smaller
	// Must in USER_TIMER_MINIMUM (0x0000000A) USER_TIMER_MAXIMUM (0x7FFFFFFF) range
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-settimer
	SetTimer(hWnd, IDT_TIMER1, 20, (TIMERPROC)NULL);

	if (*sHost && !wcschr(sHost, ' '))
	{
		swprintf_s(sCMD, MAX_PATH, L"CMD /c ping -n 1 -w 2000 %s", sHost);
		// MessageBox(hDlg, sCMD, L"", MB_OK);

		CreateProcess(NULL, sCMD, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, 2500);
		KillTimer(hWnd, IDT_TIMER1);
		SendMessage(hProgressBar, PBM_SETPOS, 100, 0);
		if (GetExitCodeProcess(pi.hProcess, &nExitCode))
		{
			if (nExitCode == 0)
			{
				// SendMessage(hIMG, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPingable);
				SendMessage(hIMG, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hPingable);
				// SendMessage(hIMG, WM_SETICON, ICON_SMALL, (LPARAM)hPingable);
			}
			else
			{
				// SendMessage(hIMG, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPingable_Not);
				SendMessage(hIMG, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hPingable_Not);
				// SendMessage(hIMG, WM_SETICON, ICON_SMALL, (LPARAM)hPingable_Not);
			}

		}
		wcscpy_s(sHost_Prev, MAX_PATH, sHost);
		CloseHandle(pi.hProcess);
	}
	else
		KillTimer(hWnd, IDT_TIMER1);

	return 0;
}
LRESULT CALLBACK DlgFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hEditHost = GetDlgItem(hDlg, IDC_EDIT_HOST);
		hIMG = GetDlgItem(hDlg, IDC_STATIC_IMG);
		hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS);
		hWnd = hDlg;
		hModule = GetModuleHandle(NULL);

		// hPingable = (HANDLE)LoadIcon(NULL, IDI_WINLOGO);
		// hPingable = ExtractIcon((HINSTANCE)hModule, L"%SystemRoot%\\System32\\SHELL32.dll", 296);
		hPingable = LoadImage(NULL, L"ACTIVE.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		// hPingable = LoadImage(NULL, L"D:\\IsHostPingable\\IsHostPingable\\ACTIVE.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		// hPingable = LoadImage(NULL, L"D:\\IsHostPingable\\IsHostPingable\\ACTIVE.bmp", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		/*if (!hPingable)
		{
			swprintf_s(sCMD, MAX_PATH, L"%d", GetLastError());
			MessageBox(hDlg, sCMD, L"", MB_OK);
		}*/
		// hPingable_Not = ExtractIcon((HINSTANCE)hModule, L"%SystemRoot%\\System32\\SHELL32.dll", 131);
		// hPingable_Not = ExtractIcon((HINSTANCE)hModule, L"HOLD.ico", 0);
		// hPingable_Not = LoadImage(NULL, L"D:\\IsHostPingable\\IsHostPingable\\HOLD.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		hPingable_Not = LoadImage(NULL, L"HOLD.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		// hPingable_Not = (HANDLE)LoadIcon(NULL, IDI_ERROR);
		hProcessing = LoadImage(NULL, L"Processing.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		break;

	case WM_COMMAND:
		/*
		SDK 10.0.17763.0 上观测到 Edit Control 已支持 Ctrl + A 全选了
		ES_MULTILINE 的仍然不会全选，跟文档中的一致

		这里 发现 Ctrl + A 会触发 EN_CHANGE，因而创建一个 sHost_Prev 比对一下，相同的就不再 ping 了
		Shift + HOME / END 仍然不会产生 EN_CHANGE
		*/
		if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_EDIT_HOST && (HWND)lParam == hEditHost)
		{
			GetWindowText((HWND)lParam, sHost, MAX_PATH);

			if (wcsicmp(sHost, sHost_Prev))
				CreateThread(NULL, 0, ThreadGUIProc, NULL, 0, NULL);

		}
		break;

	case WM_TIMER:
		SendMessage(hProgressBar, PBM_STEPIT, 0, 0);
		break;

	case WM_CLOSE:
		DestroyIcon(hPingable);
		DestroyIcon(hPingable_Not);
		EndDialog(hDlg, IDCANCEL);
	}
	return 0;
}
VOID EntryPoint()
{
	DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgFunc);

	ExitProcess(0);
}
