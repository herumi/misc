#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

#pragma comment(lib, "imagehlp.lib")

LONG CALLBACK TopLevelExceptionFilter(EXCEPTION_POINTERS *exceptionInfo)
{
	fprintf(stderr, "exception:\n");

	const int stackSize = 2048;
	int symbolBuf[stackSize / sizeof(int)];
	IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL *)symbolBuf;

	symbol->SizeOfStruct = stackSize;
	symbol->MaxNameLength = stackSize - sizeof(IMAGEHLP_SYMBOL);

	STACKFRAME sf;
	memset(&sf, 0, sizeof(sf));
	sf.AddrPC.Offset = exceptionInfo->ContextRecord->Eip;
	sf.AddrStack.Offset = exceptionInfo->ContextRecord->Esp;
	sf.AddrFrame.Offset = exceptionInfo->ContextRecord->Ebp;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;

	HANDLE processHdl = GetCurrentProcess();
	HANDLE threadHdl = GetCurrentThread();

	SymInitialize(processHdl, NULL, TRUE);

	for(;;) {
		BOOL ret;
		ret = StackWalk(IMAGE_FILE_MACHINE_I386, processHdl, threadHdl, &sf, NULL, NULL,
			SymFunctionTableAccess, SymGetModuleBase, NULL);

		if (!ret || sf.AddrFrame.Offset == 0) break;
		printf("0x%p ", sf.AddrPC.Offset);

		DWORD disp = 0;
		ret = SymGetSymFromAddr(processHdl, sf.AddrPC.Offset, &disp, symbol);
		if (ret) {
			printf("%s(+0x%p)\n", symbol->Name, disp);
		} else {
			printf("---\n");
		}
	}
	SymCleanup(processHdl);
	return EXCEPTION_EXECUTE_HANDLER;
}

void testNullAccess()
{
	*(int*)0 = 0;
}

void func1()
{
	testNullAccess();
}

void func2()
{
	func1();
}

void func3()
{
	func2();
}

int main()
{
	SetUnhandledExceptionFilter(TopLevelExceptionFilter);
	func3();
	return 0;
}