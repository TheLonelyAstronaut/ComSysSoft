#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#define STRING_LENGTH 256

int main()
{
	HANDLE semaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, L"ClientServer");
	HANDLE pipe = CreateFile(L"\\\\.\\pipe\\ClientServerPipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	char serverInput[STRING_LENGTH];
	DWORD sizeReaded;

	while (true)
	{
		if (WaitForSingleObject(semaphore, INFINITE) == WAIT_OBJECT_0)
		{
			ReadFile(pipe, serverInput, STRING_LENGTH, &sizeReaded, NULL);

			printf("%s\n", serverInput);
			
			ReleaseSemaphore(semaphore, -1, NULL);
		}
	}

	CloseHandle(semaphore);
	CloseHandle(pipe);
	system("pause");
	return 0;
}