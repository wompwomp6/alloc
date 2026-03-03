#include "peb.h"

static uintptr_t g_peb = NULL;

typedef NTSTATUS(NTAPI* pNtQueryInformationThread)(
	HANDLE,
	THREADINFOCLASS,
	PVOID,
	ULONG,
	PULONG
	);

typedef struct _THREAD_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	KAFFINITY AffinityMask;
	LONG Priority;
	LONG BasePriority;
} THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;

DWORD EnumThread(DWORD pid)
{
	THREADENTRY32 te = { 0 };
	te.dwSize = sizeof(THREADENTRY32);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (!Thread32First(hSnap, &te)) {
		return 0;
	}

	do {
		if (te.th32OwnerProcessID == pid) {
			return te.th32ThreadID;
		}

	} while (Thread32Next(hSnap, &te));
	CloseHandle(hSnap);
	return 0;
}


bool InitPeb(const wchar_t* name)
{
	DWORD tid = EnumThread(GetProcessIdByName(name));
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);
	if (!hThread)
		return false;

	HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
	if (!hNtdll)
		return false;

	pNtQueryInformationThread NtQueryInformationThread =
		(pNtQueryInformationThread)GetProcAddress(hNtdll, "NtQueryInformationThread");

	if (!NtQueryInformationThread)
		return false;

	THREAD_BASIC_INFORMATION tbi = { 0 };

	NTSTATUS status = NtQueryInformationThread(
		hThread,
		(THREADINFOCLASS)0,
		&tbi,
		sizeof(tbi),
		NULL
	);

	if (status >= 0)
	{
		ReadMem((uintptr_t)tbi.TebBaseAddress + 0x60, &g_peb, sizeof(uintptr_t));
	}
	return g_peb != NULL;
	CloseHandle(hThread);
}


bool ClosePeb()
{
	if (g_peb)
	{
		g_peb = NULL;
		return true;
	}
	return false;
}

void SetModuleSize(uintptr_t pEntry, size_t size)
{
	LDR_DATA_TABLE_ENTRY entry;
	ReadMem(pEntry, &entry, sizeof(entry));
	entry.Reserved3[1] = size;

	WriteMem(pEntry, &entry, sizeof(entry));
}

size_t GetModuleSize(uintptr_t pEntry)
{
	LDR_DATA_TABLE_ENTRY entry;
	ReadMem(pEntry, &entry, sizeof(entry));
	return entry.Reserved3[1];
}


wchar_t* GetModuleNameFromPath(const wchar_t* fullPath)
{
	if (!fullPath)
		return NULL;

	const wchar_t* lastSlash = fullPath;
	const wchar_t* ptr = fullPath;

	while (*ptr)
	{
		if (*ptr == L'\\' || *ptr == L'/')
			lastSlash = ptr + 1;

		ptr++;
	}

	return (wchar_t*)lastSlash;
}

uintptr_t GetModuleEntryByName(wchar_t* mod)
{
	if (!g_peb) return NULL;
	PEB remotePeb;
	ReadMem(g_peb, &remotePeb, sizeof(remotePeb));

	PEB_LDR_DATA remoteLdr;
	ReadMem((uintptr_t)remotePeb.Ldr, &remoteLdr, sizeof(remoteLdr));

	uintptr_t listHead = (uintptr_t)remotePeb.Ldr +
		offsetof(PEB_LDR_DATA, InMemoryOrderModuleList);

	LIST_ENTRY head;
	ReadMem(listHead, &head, sizeof(head));

	uintptr_t current = (uintptr_t)head.Flink;
	do
	{
		LDR_DATA_TABLE_ENTRY entry;

		uintptr_t entryAddress = current - offsetof(LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		ReadMem(entryAddress, &entry, sizeof(entry));

		WCHAR nameBuffer[260] = { 0 };
		ReadMem(entry.FullDllName.Buffer, nameBuffer, entry.FullDllName.Length);

		wchar_t* modulename = GetModuleNameFromPath(nameBuffer);
		if (!wcscmp(modulename, mod)) return entryAddress;
		
		current = (uintptr_t)entry.InMemoryOrderLinks.Flink;
	} while (current != listHead);
	return NULL;
}