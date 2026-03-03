#include "alloc/alloc.h"

int main()
{
	if(!InitAlloc(L"cs2.exe"))
		printf("Failed to initialize allocator.\n");
	
	const uintptr_t p = Alloc(L"cs2.exe", MEM_PAGE, PAGE_READWRITE);//PAGE_EXECUTE_READWRITE

	printf("%p\n", p);
	while (!GetAsyncKeyState(VK_END)) { Sleep(10); }
	
	if (!Free(L"cs2.exe", MEM_PAGE))
		printf("Failed to free memory.\n");
	
	CloseAlloc();
}