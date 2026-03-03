#pragma once
#ifndef PEB_H
#define PEB_H

#include "../types.h"
#include "../mem/mem.h"

#include <Windows.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

#ifdef __cplusplus
extern "C" {
#endif

extern uintptr_t g_peb;


bool InitPeb(const wchar_t* name);
bool ClosePeb();

void SetModuleSize(uintptr_t pEntry, size_t size);
size_t GetModuleSize(uintptr_t pEntry);

uintptr_t GetModuleEntryByName(const wchar_t* mod);

#ifdef __cplusplus
}
#endif

#endif