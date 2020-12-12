#include <Windows.h>
#include "resource.h"
#include <iostream>
#include <psapi.h>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <sstream>
#include <tchar.h>

using namespace std;
using namespace std::chrono;

INT_PTR CALLBACK MainDlgProc(HWND, UINT, WPARAM,LPARAM);
INT_PTR CALLBACK ManagerDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);
LRESULT CALLBACK KeyBoard(int iCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseMove(int iCode, WPARAM wParam, LPARAM lParam);

HBRUSH hPixelBrush;
HDC hdcPixelColor;

COLORREF color;
HWND windowDesc;
HWND hwProcessName;
HWND hwProcessWay;
HWND hwPid;
HWND hwWindowHwnd;
HWND hwPosX;
HWND hwPosY;
HWND hwColor;
HWND hwMainDialog;
HWND hwRGB;
HWND hwStatus;
HINSTANCE hiMain;

HHOOK keyBoardHook;
HHOOK mouseHook;

HWND hWindowTitele;
HWND hWinPosX;
HWND hWinPosY;
HWND hWinHeight;
HWND hWinWidth;
HWND hWinTrans;
HWND hSelectWindow;
HWND hAllWindow;
vector<char*> winNames;
vector<HWND> winHwnd;

UINT MainTimer = 322;
BOOL StartTrace = FALSE;
BOOL moveFlag = FALSE;

vector<string> recordCommands;
BOOL RecordFlag = FALSE;

milliseconds newTime, oldTime;

HANDLE executeThread;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	hiMain = hInstance;
	hwMainDialog = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, (MainDlgProc), 0);
	HMENU hmenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	
	SetMenu(hwMainDialog, hmenu);
	SetTimer(hwMainDialog, MainTimer, 100,NULL);

	ShowWindow(hwMainDialog, nCmdShow);
	UpdateWindow(hwMainDialog);
	
	MSG msg;
	
	keyBoardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyBoard, NULL, NULL);
	mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseMove, NULL, NULL);

	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(hwMainDialog, hAccel, &msg))
		{
			if (!IsDialogMessage(hwMainDialog, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	DestroyMenu(hmenu);
	KillTimer(hwMainDialog, MainTimer);

	UnhookWindowsHookEx(keyBoardHook);
	UnhookWindowsHookEx(mouseHook);
	return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char* str = (char*)malloc(255);
	memset(str, 0, 255);
	if (GetWindowText(hwnd, str, 255)) {
		if (IsWindowVisible(hwnd) && (!GetWindow(hwnd, GW_OWNER)))
		{
			winNames.push_back(str);
			winHwnd.push_back(hwnd);
		}
	}
	return TRUE;	
}

DWORD WINAPI ChangeColor(HWND h)
{
	if (hPixelBrush) {
		DeleteObject(hPixelBrush);
		hPixelBrush = NULL;
	}
	InvalidateRect(h, NULL, TRUE);
	return 0;
}

void GetRGBString(COLORREF colorLocal, char* rgbColor) {
	DWORD red = GetRValue(colorLocal);
	DWORD green = GetGValue(colorLocal);
	DWORD blue = GetBValue(colorLocal);

	char bufRedColor[11] = { 0 };
	char bufGreenColor[11] = { 0 };
	char bufBlueColor[11] = { 0 };

	_ultoa_s(red, bufRedColor, 10);
	strcat(rgbColor, bufRedColor);
	strcat(rgbColor, " ");

	_ultoa_s(green, bufGreenColor, 10);
	strcat(rgbColor, bufGreenColor);
	strcat(rgbColor, " ");

	_ultoa_s(blue, bufBlueColor, 10);
	strcat(rgbColor, bufBlueColor);
}

void WINAPI ChangeWindowsInfo() {
	winNames.clear();
	winHwnd.clear();
	SendMessage(hAllWindow, CB_RESETCONTENT, 0, 0);
	EnumWindows(&EnumWindowsProc, 0);
	for (int i = 0; i < winNames.size(); i++) {
		SendMessage(hAllWindow, CB_ADDSTRING, 0, (LPARAM)(winNames[i]));
	}
	SendMessage(hAllWindow, CB_SETCURSEL, (WPARAM)0, 0);
	char selectCh[11] = { 0 };

	_ultoa_s((DWORD)winNames[0], selectCh, 10);
	SetWindowText(hSelectWindow, selectCh);
}

DWORD WINAPI DoRecordComand(LPVOID lpParam) {
	while (1) {
		ifstream in("log.txt");
		if (in.is_open())
		{
			string line;
			while (getline(in, line))
			{
				string buf;
				int x, y;
				if (line.find("keyDown:") != string::npos) {
					for (int i = 8; i < line.size(); i++) {
						buf += line[i];
					}
					std::stringstream geek(buf);
					int x = 0;
					geek >> x;
					keybd_event(x, 0, 0, 0);
				}
				if (line.find("keyUp:") != string::npos) {
					for (int i = 6; i < line.size(); i++) {
						buf += line[i];
					}
					std::stringstream geek(buf);
					int x = 0;
					geek >> x;
					keybd_event(x, 0, KEYEVENTF_KEYUP, 0);
				}
				if (line.find("move:") != string::npos) {
					for (int i = 5; i < line.size(); i++) {
						if (line[i] == ';') {
							std::stringstream geek(buf);
							geek >> x;
							buf = "";
						}
						else {
							buf += line[i];
						}
					}
					std::stringstream geek(buf);
					geek >> y;
					SetCursorPos(x, y);
				}
				if (line.find("rButtonDown:") != string::npos) {
					for (int i = 12; i < line.size(); i++) {
						if (line[i] == ';') {
							std::stringstream geek(buf);
							geek >> x;
							buf = "";
						}
						else {
							buf += line[i];
						}
					}
					std::stringstream geek(buf);
					geek >> y;
					SetCursorPos(x, y);
					mouse_event(MOUSEEVENTF_RIGHTDOWN, x, y, 0, 0);
				}
				if (line.find("rButtonUp:") != string::npos) {
					for (int i = 10; i < line.size(); i++) {
						if (line[i] == ';') {
							stringstream geek(buf);
							geek >> x;
							buf = "";
						}
						else {
							buf += line[i];
						}
					}
					stringstream geek(buf);
					geek >> y;
					SetCursorPos(x, y);
					mouse_event(MOUSEEVENTF_RIGHTUP, x, y, 0, 0);
				}
				if (line.find("lButtonDown:") != string::npos) {
					for (int i = 12; i < line.size(); i++) {
						if (line[i] == ';') {
							std::stringstream geek(buf);
							geek >> x;
							buf = "";
						}
						else {
							buf += line[i];
						}
					}
					std::stringstream geek(buf);
					geek >> y;
					SetCursorPos(x, y);
					mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, 0, 0);
				}
				if (line.find("lButtonUp:") != string::npos) {
					for (int i = 10; i < line.size(); i++) {
						if (line[i] == ';') {
							stringstream geek(buf);
							geek >> x;
							buf = "";
						}
						else {
							buf += line[i];
						}
					}
					stringstream geek(buf);
					geek >> y;
					SetCursorPos(x, y);
					mouse_event(MOUSEEVENTF_LEFTUP, x, y, 0, 0);
				}
				if (line.find("sleep:") != string::npos) {
					for (int i = 6; i < line.size(); i++) {
						buf += line[i];
					}
					stringstream geek(buf);
					int x = 0;
					geek >> x;
					Sleep(x);
				}
			}
			in.close();
		}
	}
	return 0;
}
void WINAPI GetSelectedInfo(int num) {
	WINDOWINFO windowInfo; 
	windowInfo.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(winHwnd[num], &windowInfo);

	SetWindowText(hWindowTitele, winNames[num]);

	char bufStr[11] = { 0 };

	_ultoa_s((DWORD)winHwnd[num], bufStr, 10);
	SetWindowText(hSelectWindow, bufStr);

	_itoa_s(windowInfo.rcWindow.left, bufStr,10);
	SetWindowText(hWinPosX, bufStr);
	memset(bufStr, 0,11);

	_itoa_s(windowInfo.rcWindow.top, bufStr, 10);
	SetWindowText(hWinPosY, bufStr);
	memset(bufStr, 0, 11);

	long height = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;
	_itoa_s(height, bufStr, 10);
	SetWindowText(hWinHeight, bufStr);
	memset(bufStr, 0, 11);

	long width = windowInfo.rcWindow.right - windowInfo.rcWindow.left;
	_itoa_s(width, bufStr, 10);
	SetWindowText(hWinWidth, bufStr);
	memset(bufStr, 0, 11);
}

void WINAPI ChangeWindowPror() {
	
	int j = SendMessage(hAllWindow, CB_GETCURSEL, 0, 0);
	char buf[11] = { 0 };

	GetWindowText(hWinPosX, buf, 10);
	int x = atoi(buf);
	memset(buf, 0, 11); 

	GetWindowText(hWinPosY, buf, 10);
	int y = atoi(buf);
	memset(buf, 0, 11);
	
	GetWindowText(hWinWidth, buf, 10);
	int width = atoi(buf);
	memset(buf, 0, 11);

	GetWindowText(hWinHeight, buf, 10);
	int heigh = atoi(buf);
	memset(buf, 0, 11);

	int length = GetWindowTextLength(hWindowTitele) + 1;
	char* bufTitle = (char*)malloc(length);
	memset(bufTitle, 0, length);
	GetWindowText(hWindowTitele, bufTitle, length);
	SetWindowText(winHwnd[j], bufTitle);
	free(bufTitle);

	SetWindowPos(winHwnd[j], HWND_TOP, x, y, width, heigh, SWP_SHOWWINDOW);

	SetWindowLong(winHwnd[j], GWL_EXSTYLE, GetWindowLong(winHwnd[j], GWL_EXSTYLE) | WS_EX_LAYERED);
	int k = SendMessage(hWinTrans, CB_GETCURSEL, 0, 0);
	SetLayeredWindowAttributes(winHwnd[j], 0, 255 * (float)(10 - k) / 10, LWA_ALPHA);
}

INT_PTR  CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg)
	{
	case WM_MOUSEMOVE:

		break;
	case WM_INITDIALOG:
		hwProcessName = GetDlgItem(hWnd, IDC_PROCESS_NAME);
		hwProcessWay = GetDlgItem(hWnd, IDC_PROCESS_WAY);
		hwPid = GetDlgItem(hWnd, IDC_PID);
		hwWindowHwnd = GetDlgItem(hWnd, IDC_HWND);
		hwPosX = GetDlgItem(hWnd, IDC_POSX);
		hwPosY = GetDlgItem(hWnd, IDC_POSY);
		hwColor = GetDlgItem(hWnd, IDC_COLOR);
		hwRGB = GetDlgItem(hWnd, IDC_RGB);
		hwStatus = GetDlgItem(hWnd, IDC_STATUS);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			return TRUE;
		case WINDOW_MANAGER:
			DialogBoxParam(hiMain, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, (ManagerDlgProc), 0);
			break;
		case ID_QUIT:
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			return TRUE;
		}
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hwColor)
		{
			hdcPixelColor = reinterpret_cast<HDC>(wParam);
			SetBkColor(hdcPixelColor, color);
			if (!hPixelBrush)
			{
				hPixelBrush = CreateSolidBrush(color);
			}
			return reinterpret_cast<LRESULT>(hPixelBrush);
		}
		break;
	case WM_TIMER:
		POINT cursorPoint;
		GetCursorPos(&cursorPoint);

		windowDesc = WindowFromPoint(cursorPoint);

		HDC DesktopDC = GetDC(NULL);
		color = GetPixel(DesktopDC, cursorPoint.x, cursorPoint.y);
		ReleaseDC(NULL, DesktopDC);

		char rgbString[34] = { 0 };
		GetRGBString(color, rgbString);

		char posX[11] = { 0 };
		char posY[11] = { 0 };

		_itoa_s(cursorPoint.x, posX, 10);
		_itoa_s(cursorPoint.y, posY, 10);

		char windowDescCh[11] = { 0 };
		char processIDCh[11] = { 0 };

		DWORD processID = 0;

		GetWindowThreadProcessId(windowDesc, &processID);

		_ultoa_s((DWORD)windowDesc, windowDescCh, 10);
		_ultoa_s(processID, processIDCh, 10);

		DWORD waySize = MAX_PATH;
		char fullProcessWay[MAX_PATH] = { 0 };

		HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
		if (processHandle != NULL)
		{
			QueryFullProcessImageName(processHandle, 0, fullProcessWay, &waySize);
		}
		CloseHandle(processHandle);

		int i = size(fullProcessWay);
		while (fullProcessWay[i] != '\\' && i != 0) {
			i--;
		}

		int nameSize = size(fullProcessWay) - i + 1;
		char * processName = (char*)malloc(nameSize);

		memset(processName, 0, nameSize);

		for (int j = 0; j < nameSize; j++) {
			i++;
			processName[j] = fullProcessWay[i];
		}

		SetWindowText(hwProcessName, processName);
		SetWindowText(hwProcessWay, fullProcessWay);
		SetWindowText(hwPid, processIDCh);
		SetWindowText(hwWindowHwnd, windowDescCh);
		SetWindowText(hwPosX, posX);
		SetWindowText(hwPosY, posY);
		SetWindowText(hwRGB, rgbString);

		ChangeColor(hwColor);
		free(processName);
		break;
	}
	return FALSE;
}

INT_PTR  CALLBACK ManagerDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_INITDIALOG:
			hAllWindow = GetDlgItem(hWnd, IDC_ALLWINDOW);
			hSelectWindow = GetDlgItem(hWnd, IDC_SELECT);
			hWindowTitele = GetDlgItem(hWnd, IDC_WINDOWTITLE);
			hWinPosX = GetDlgItem(hWnd, IDC_WINPOSX);
			hWinPosY = GetDlgItem(hWnd, IDC_WINPOSY);
			hWinHeight = GetDlgItem(hWnd, IDC_HEIGHT);
			hWinWidth = GetDlgItem(hWnd, IDC_WIDTH);
			hWinTrans = GetDlgItem(hWnd, IDC_TRANS);

			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("100 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("90 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("80 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("70 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("60 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("50 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("40 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("30 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("20 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("10 %"));
			SendMessage(hWinTrans, CB_ADDSTRING, 0, (LPARAM)("0 %"));
			SendMessage(hWinTrans, CB_SETCURSEL, (WPARAM)0, 0);

			ChangeWindowsInfo();
			GetSelectedInfo(0);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_ALLWINDOW:
					if (HIWORD(wParam) == CBN_SELENDOK) {
						SendMessage(hWinTrans, CB_SETCURSEL, (WPARAM)0, 0);
						int j = SendMessage(hAllWindow, CB_GETCURSEL, 0, 0);
						GetSelectedInfo(j); 
					}
					break;
				case IDC_REFRESH:
					ChangeWindowsInfo();
					GetSelectedInfo(0);
					break;
				case IDC_ACCEPT:
					ChangeWindowPror();
					break;
				case IDC_DESTROY:
					int j = SendMessage(hAllWindow, CB_GETCURSEL, 0, 0);
					SendMessage(winHwnd[j], WM_SYSCOMMAND, SC_CLOSE,0);

					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return FALSE;
	}
	return FALSE;
}

LRESULT CALLBACK KeyBoard(_In_ int iCode, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	DWORD SHIFT_key = 0;
	DWORD CTRL_key = 0;
	DWORD ALT_key = 0;
	KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam);
	
	int key = hooked_key.vkCode;
	
	SHIFT_key = GetAsyncKeyState(VK_SHIFT);
	CTRL_key = GetAsyncKeyState(VK_CONTROL);
	ALT_key = GetAsyncKeyState(VK_MENU);
	if (RecordFlag) {
		DWORD delay = 0;
		newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		delay = (newTime - oldTime).count();
		oldTime = newTime;
		recordCommands.push_back("sleep:" + to_string(delay) + "\n");
		if ((iCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
		{
			recordCommands.push_back("keyDown:" + to_string(key) + "\n");
		}
		if ((iCode == HC_ACTION) && ((wParam == WM_SYSKEYUP) || (wParam == WM_KEYUP))) {

			recordCommands.push_back("keyUp:" + to_string(key) + "\n");
		}
	}
	if ((iCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
	{

		if (ALT_key != 0 && key == 'T')
		{
			SendMessage(windowDesc, WM_SYSCOMMAND, SC_CLOSE, 0);
		}
		if (ALT_key != 0 && key == 'X')
		{
			SetWindowText(hwStatus, "WAITING FOR RECORDING");
			TerminateThread(executeThread, 5);
		}
		if (ALT_key != 0 && key == 'G')
		{
			SetWindowText(hwStatus, "REPLAY");
			executeThread = CreateThread(NULL,0, DoRecordComand,NULL,0,NULL);
			DWORD status;
			while (1) {
				GetExitCodeThread(executeThread, &status);
				if (status == 0) {
					break;
				}
				if (status == 5) {
					MessageBox(NULL, "HAD", "ASD", MB_OK);
					break;
				}
			}
			//WaitForSingleObject(executeThread, INFINITE);
			CloseHandle(executeThread);
		}
		if (ALT_key != 0 && key == 'R')
		{
			RecordFlag = true;
			oldTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			recordCommands.clear();
			SetWindowText(hwStatus, "RECORDING");
		}
		if (ALT_key != 0 && key == 'S')
		{
			if (RecordFlag == true) {
				SetWindowText(hwStatus, "WAITING FOR RECORDING");
				RecordFlag = false;
				std::ofstream out;
				out.open("log.txt");
				for (int i = 4; i < recordCommands.size() - 4; i++) {
					out << recordCommands[i];
				}
				out.close();
				recordCommands.clear();
			}
		}
	}
	return CallNextHookEx(keyBoardHook, iCode, wParam, lParam);
}

LRESULT CALLBACK MouseMove(_In_ int iCode, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	
	if (RecordFlag && iCode >= 0) {
		PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;
		DWORD delay = 0;
		switch (wParam)
		{
			case WM_MOUSEMOVE:
				newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
				delay = (newTime - oldTime).count();
				if (delay > 5) {
					recordCommands.push_back("sleep:" + to_string(delay) + "\n");
					recordCommands.push_back("move:" + to_string(p->pt.x) + ";" + to_string(p->pt.y) + "\n");
					oldTime = newTime;
				}
				break;
			case WM_RBUTTONDOWN:
				newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			    delay = (newTime - oldTime).count();
				oldTime = newTime;
				recordCommands.push_back("sleep:" + to_string(delay) + "\n");
				recordCommands.push_back("rButtonDown:" + to_string(p->pt.x) + ";" + to_string(p->pt.y) + "\n");
				break;
			case WM_RBUTTONUP:
				newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
				delay = (newTime - oldTime).count();
				oldTime = newTime;
				recordCommands.push_back("sleep:" + to_string(delay) + "\n");
				recordCommands.push_back("rButtonUp:" + to_string(p->pt.x) + ";" + to_string(p->pt.y) + "\n");
				break;
			case WM_LBUTTONDOWN:
				newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
				delay = (newTime - oldTime).count();
				oldTime = newTime;
				recordCommands.push_back("sleep:" + to_string(delay) + "\n");
				recordCommands.push_back("lButtonDown:" + to_string(p->pt.x) + ";" + to_string(p->pt.y) + "\n");
				break;
			case WM_LBUTTONUP:
				newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
				delay = (newTime - oldTime).count();
				oldTime = newTime;
				recordCommands.push_back("sleep:" + to_string(delay) + "\n");
				recordCommands.push_back("lButtonUp:" + to_string(p->pt.x) + ";" + to_string(p->pt.y) + "\n");
				break;
			}
	}
	return CallNextHookEx(mouseHook, iCode, wParam, lParam);
}