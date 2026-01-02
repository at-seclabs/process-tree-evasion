#include "proc.h"
#include <iostream>

BOOL EnableDebugPrivilege()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        return FALSE;

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}

DWORD GetProcId(const wchar_t* procName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    DWORD oldestPid = 0;
    FILETIME oldestTime = { 0 };
    oldestTime.dwHighDateTime = MAXDWORD;  // Temps le plus récent possible
    oldestTime.dwLowDateTime = MAXDWORD;

    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            if (_wcsicmp(pe32.szExeFile, procName) == 0)
            {
                // Ouvrir le processus pour vérifier son temps de création
                HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
                if (hProc)
                {
                    FILETIME createTime, exitTime, kernelTime, userTime;

                    if (GetProcessTimes(hProc, &createTime, &exitTime, &kernelTime, &userTime))
                    {
                        // Si ce processus est plus ancien que le précédent
                        if (CompareFileTime(&createTime, &oldestTime) < 0)
                        {
                            oldestTime = createTime;
                            oldestPid = pe32.th32ProcessID;
                        }
                    }

                    CloseHandle(hProc);
                }
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (oldestPid == 0)
    {
        printf("[-] No accessible instance of %ls found\n", procName);
    }
    else
    {
        printf("[+] Selected oldest instance (PID: %lu)\n", oldestPid);
    }

    return oldestPid;
}

HANDLE GetParentProcHandle(DWORD parentProcId)
{
    // Request both PROCESS_CREATE_PROCESS and PROCESS_QUERY_INFORMATION rights
    HANDLE hParent = OpenProcess(
        PROCESS_CREATE_PROCESS | PROCESS_QUERY_INFORMATION,
        FALSE,
        parentProcId
    );

    if (hParent == NULL)
    {
        DWORD error = GetLastError();
        wprintf(L"[-] Failed to open process PID %lu: error %lu\n", parentProcId, error);

        // Provide specific error guidance
        switch (error)
        {
        case ERROR_ACCESS_DENIED:  // 5
            wprintf(L"[-] Access denied - run as administrator or enable SeDebugPrivilege\n");
            break;
        case ERROR_INVALID_PARAMETER:  // 87
            wprintf(L"[-] Invalid PID - process may have terminated\n");
            break;
        default:
            wprintf(L"[-] Unknown error\n");
        }
    }

    return hParent;
}


BOOL PrepareStartupInfoEx(HANDLE hParent, STARTUPINFOEX* sie, SIZE_T* attributeSize) {
    // get necessary size
    InitializeProcThreadAttributeList(NULL, 1, 0, attributeSize);

    // allocate mem
    sie->lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
        GetProcessHeap(),
        0,
        *attributeSize
    );

    if (sie->lpAttributeList == NULL) {
		std::cout << "Heap allocation error: " << GetLastError() << std::endl;
        return FALSE;
    }

    // Initialize list
    if (!InitializeProcThreadAttributeList(
        sie->lpAttributeList,
        1,
        0,
        attributeSize
    )) {
		std::cout << "Error InitializeProcThreadAttributeList: " << GetLastError() << std::endl;
        HeapFree(GetProcessHeap(), 0, sie->lpAttributeList);
        return FALSE;
    }

    // Struct configuration
    sie->StartupInfo.cb = sizeof(STARTUPINFOEX);

    return TRUE;
}