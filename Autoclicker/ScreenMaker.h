#pragma once
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP);
void CreateBMPFile(LPTSTR pszFile, HBITMAP hBMP);