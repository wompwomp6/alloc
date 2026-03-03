#include "alloc.h"

bool InitAlloc(const wchar_t* processName)
{
    if (!processName) return false;

    bool s = true;
    if (!InitMemory(processName)) s = false;
	if (!InitPeb(processName)) s = false;
	return s;
}
void CloseAlloc()
{
	ClosePeb();
    CloseMemory();
}

void align8(size_t* size)
{
    if (!size) return;
    *size = (*size + 7) & ~7ULL;
}

uintptr_t Alloc(wchar_t* mod, size_t size, DWORD protection)
{
    const uintptr_t entry = GetModuleEntryByName(mod);
    if (!entry) return NULL;

    size_t modSize = GetModuleSize(entry);
    align8(&size);

    for (size_t i = 0; (i / MEM_PAGE) < MAX_PAGE_SEARCH; i += MEM_PAGE)
    {
        uintptr_t r = AllocateMEM((GetModuleBase(mod) + modSize) + i, size, protection);
        if (!r) continue;

        SetModuleSize(entry, modSize + size + i);
        return r;
    }

    return NULL;
}

bool Free(wchar_t* mod, size_t size)
{
    const uintptr_t entry = GetModuleEntryByName(mod);
    if (!entry) return NULL;
    size_t modSize = GetModuleSize(entry);
    align8(&size);

    uintptr_t allocAddr = (GetModuleBase(mod) + modSize) - size;

	printf("Freeing memory at %p\n", allocAddr);
    while (!GetAsyncKeyState(VK_END)) { Sleep(10); }

    if (!FreeMem_s(allocAddr, NULL/*size*/))return false;

    SetModuleSize(entry, modSize - size);

    return true;
}