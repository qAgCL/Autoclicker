#include <Windows.h>
#include "resource.h"
#include <iostream>
#include <psapi.h>

using namespace std;
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM,LPARAM);


HBRUSH hPixelBrush;
HDC hdcPixelColor;

COLORREF color;
HWND hwProcessName;
HWND hwProcessWay;
HWND hwPid;
HWND hwWindowHwnd;
HWND hwPosX;
HWND hwPosY;
HWND hwColor;
HWND hwMainDialog;
HWND hwRGB;

HHOOK hHook;

LRESULT CALLBACK MouseKey(int iCode, WPARAM wParam, LPARAM lParam);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	hHook = SetWindowsHookEx(WH_MOUSE_LL, MouseKey, NULL, NULL);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, (DlgProc), 0);
	UnhookWindowsHookEx(hHook);
	return 0;
}

INT_PTR  CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	hwMainDialog = hWnd;
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
			break;
		case WM_COMMAND:
			break;
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
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return FALSE;
	}
	return FALSE;
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

void GetRGBString(COLORREF colorLocal,char* rgbColor) {
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
LRESULT CALLBACK MouseKey(int iCode, WPARAM wParam, LPARAM lParam) {
	if (iCode >= 0 && wParam == WM_MOUSEMOVE) {

		POINT cursorPoint;
		HWND windowDesc;
		
		GetCursorPos(&cursorPoint);
		windowDesc = WindowFromPoint(cursorPoint);

		HDC DesktopDC = GetDC(NULL);
		color = GetPixel(DesktopDC, cursorPoint.x, cursorPoint.y);
		ReleaseDC(NULL, DesktopDC);

		char rgbString[34] = { 0 };
		GetRGBString(color,rgbString);

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

		HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, processID);
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

		for (int j=0; j < nameSize; j++) {
			i++;
			processName[j] = fullProcessWay[i];
		}

		
		SetWindowText(hwProcessName, processName);
		SetWindowText(hwProcessWay,fullProcessWay);
		SetWindowText(hwPid,processIDCh);
		SetWindowText(hwWindowHwnd,windowDescCh);
		SetWindowText(hwPosX,posX);
		SetWindowText(hwPosY,posY);
		SetWindowText(hwRGB, rgbString);

		SendMessage(hwMainDialog, WM_CTLCOLORSTATIC,0, (LPARAM)hwColor);
		ChangeColor(hwColor);
		free(processName);
	}

	return CallNextHookEx(hHook, iCode, wParam, lParam);
}