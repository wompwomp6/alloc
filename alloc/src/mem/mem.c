#pragma once
#include "mem.h"

static DWORD  g_processId = 0;
static HANDLE g_hProc = NULL;

DWORD GetProcessIdByName(const wchar_t* name)
{
    DWORD pid = 0;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snap, &entry))
    {
        do
        {
            if (!_wcsicmp(entry.szExeFile, name))
            {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &entry));
    }

    CloseHandle(snap);
    return pid;
}

bool InitMemory(const wchar_t* processName)
{
    g_processId = GetProcessIdByName(processName);
    if (!g_processId)
        return false;

    g_hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_processId);
    if (!g_hProc)
        return false;

    return true;
}

void CloseMemory()
{
    if (g_hProc)
    {
        CloseHandle(g_hProc);
        g_hProc = NULL;
    }
}

bool ReadMem(uintptr_t address, void* buffer, size_t size)
{
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(
        g_hProc,
        (LPCVOID)address,
        buffer,
        size,
        &bytesRead
    ) && bytesRead == size;
}

bool WriteMem(uintptr_t address, const void* buffer, size_t size)
{
    SIZE_T bytesWritten = 0;
    return WriteProcessMemory(
        g_hProc,
        (LPVOID)address,
        buffer,
        size,
        &bytesWritten
    ) && bytesWritten == size;
}

uintptr_t GetModuleBase(const wchar_t* moduleName)
{
    uintptr_t base = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        g_processId
    );

    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32W moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);

    if (Module32FirstW(snapshot, &moduleEntry))
    {
        do
        {
            if (!_wcsicmp(moduleEntry.szModule, moduleName))
            {
                base = (uintptr_t)moduleEntry.modBaseAddr;
                break;
            }
        } while (Module32NextW(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return base;
}

uintptr_t AllocateMEM(uintptr_t address, size_t size, DWORD protection)
{
    LPVOID addr = VirtualAllocEx(
        g_hProc,
        (LPVOID)address,
        size,
        MEM_COMMIT | MEM_RESERVE,
        protection
    );

    return (uintptr_t)addr;
}

bool FreeMem(uintptr_t address) { FreeMem_s(address, NULL); }

bool FreeMem_s(uintptr_t address, size_t size)
{
    return VirtualFreeEx(
        g_hProc,
        (LPVOID)address,
        size,
        MEM_RELEASE
    );
}

bool ChangeProtection(uintptr_t address, SIZE_T size, DWORD newProtect, DWORD* oldProtect)
{
    return VirtualProtectEx(
        g_hProc,
        (LPVOID)address,
        size,
        newProtect,
        oldProtect
    );
}

bool IsAllocated(uintptr_t address)
{
    if (!g_hProc)
        return false;

    MEMORY_BASIC_INFORMATION mbi;
    SIZE_T result = VirtualQueryEx(
        g_hProc,
        (LPCVOID)address,
        &mbi,
        sizeof(mbi)
    );

    if (!result)
        return false;

    return mbi.State == MEM_COMMIT;
}