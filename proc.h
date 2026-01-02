#pragma once
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

BOOL EnableDebugPrivilege();

DWORD GetProcId(const wchar_t* procName);

HANDLE GetParentProcHandle(DWORD parentProcId);

BOOL PrepareStartupInfoEx(HANDLE hParent, STARTUPINFOEX* sie, SIZE_T* attributeSize);