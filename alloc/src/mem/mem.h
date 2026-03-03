#pragma once
#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <stdint.h>
#include <TlHelp32.h>
#include <wchar.h>

#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_PAGE 0x1000

extern DWORD  g_processId;
extern HANDLE g_hProc;

DWORD GetProcessIdByName(const wchar_t* name);

bool  InitMemory(const wchar_t* processName);
void CloseMemory(void);

bool ReadMem(uintptr_t address, void* buffer, size_t size);
bool WriteMem(uintptr_t address, const void* buffer, size_t size);

uintptr_t GetModuleBase(const wchar_t* moduleName);

bool ChangeProtection(uintptr_t address, SIZE_T size, DWORD newProtect, DWORD* oldProtect);
//bool IsAllocated(uintptr_t address);

uintptr_t AllocateMEM(uintptr_t address, size_t size, DWORD protection);
bool FreeMem(uintptr_t address);
bool FreeMem_s(uintptr_t address, size_t size);

#ifdef __cplusplus
}
#endif

#endif