#pragma once
#ifndef ALLOC_H
#define ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../peb/peb.h"

#define MAX_PAGE_SEARCH 1000

bool InitAlloc(const wchar_t* processName);
void CloseAlloc();

uintptr_t Alloc(wchar_t* mod, size_t size, DWORD protection);
bool Free(wchar_t* mod, size_t size);
#ifdef __cplusplus
}
#endif

#endif