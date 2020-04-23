#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <string>
#include <conio.h>
#include <iostream>
#define FILE_SIZE 3
using namespace std;

unsigned int __stdcall ReadFile(void* data);
unsigned int __stdcall WriteToFile(void* data);
LPCWSTR files[FILE_SIZE] = { L"D:\\Programming\\ComSysSoft\\Lab5\\Windows\\Debug\\inp1.txt", L"D:\\Programming\\ComSysSoft\\Lab5\\Windows\\Debug\\inp2.txt", L"D:\\Programming\\ComSysSoft\\Lab5\\Windows\\Debug\\inp3.txt" };
CRITICAL_SECTION critical;

struct ReaderWriter
{
	HANDLE readFileDescriptor;
	HANDLE writeFileDescriptor;
	DWORD byteNumber;
	CHAR buffer[200];
	DWORD outPosition;
	OVERLAPPED overlapped;
} asyncObject;

HINSTANCE dynamicLibrary;


int main(int argc, char* argv[])
{
	InitializeCriticalSection(&critical);
	HANDLE overlapEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("OverlapEvent"));

	OVERLAPPED overlapped;
	overlapped.hEvent = overlapEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = NULL;

	asyncObject.overlapped = overlapped;
	asyncObject.outPosition = 0;
	HANDLE outFile = CreateFile(L"D:\\Programming\\ComSysSoft\\Lab5\\Windows\\Debug\\out.txt", GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	asyncObject.writeFileDescriptor = outFile;
	asyncObject.byteNumber = sizeof(asyncObject.buffer);

	HANDLE threads[2];

	dynamicLibrary = LoadLibrary(L"AsyncOperations.dll");

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ReadFile, 0, 0, NULL);
	Sleep(100);
	threads[1] = (HANDLE)_beginthreadex(NULL, 0, WriteToFile, 0, 0, NULL);
	WaitForMultipleObjects(2, threads, TRUE, INFINITE);

	FreeLibrary(dynamicLibrary);

	CloseHandle(overlapEvent);
	CloseHandle(threads[0]);
	CloseHandle(threads[1]);
	DeleteCriticalSection(&critical);
	return 0;
}


unsigned int __stdcall ReadFile(void* data)
{
	BOOL(*Read)(ReaderWriter*) = (BOOL(*)(ReaderWriter*))GetProcAddress(dynamicLibrary, "AsyncRead");

	for (int i = 0; i < FILE_SIZE; i++)
	{
		EnterCriticalSection(&critical);
		asyncObject.readFileDescriptor = CreateFile(files[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		(Read)(&asyncObject);
		CloseHandle(asyncObject.readFileDescriptor);
		LeaveCriticalSection(&critical);
		Sleep(100);
	}
	return 0;
}

unsigned int __stdcall WriteToFile(void* data)
{
	BOOL(*Write)(ReaderWriter*) = (BOOL(*)(ReaderWriter*))GetProcAddress(dynamicLibrary, "AsyncWrite");

	for (int i = 0; i < FILE_SIZE; i++)
	{
		EnterCriticalSection(&critical);
		(Write)(&asyncObject);
		LeaveCriticalSection(&critical);
		Sleep(100);
	}

	CloseHandle(asyncObject.writeFileDescriptor);
	return 0;
}