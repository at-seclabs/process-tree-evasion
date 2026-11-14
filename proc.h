#pragma once
#include <Windows.h>

BOOL EnableDebugPrivilege();

DWORD GetProcId(const wchar_t* procName);

HANDLE GetParentProcHandle(DWORD parentProcId);
