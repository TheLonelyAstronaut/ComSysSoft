#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#define STRING_LENGTH 256

void createProcess(TCHAR* path, PROCESS_INFORMATION& processInfo);

int _tmain(int argc, TCHAR* argv[])
{
    PROCESS_INFORMATION processInfo;
    DWORD sizeWritten;
    HANDLE semaphore = CreateSemaphore(NULL, 0, 1, L"ClientServer");
    HANDLE pipe = CreateNamedPipe(L"\\\\.\\pipe\\ClientServerPipe", 
                                    PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                    PIPE_UNLIMITED_INSTANCES, 
                                    512, 512, 5000, NULL);
  
    char serverInput[STRING_LENGTH];

    createProcess(argv[1], processInfo);
    ConnectNamedPipe(pipe, NULL);

    system("cls");
    

    while (true)
    {
        std::cout << "Server input: ";
        gets_s(serverInput);
        
        WriteFile(pipe, serverInput, strlen(serverInput) + 1, &sizeWritten, NULL);
        
        ReleaseSemaphore(semaphore, 1, NULL);
        system("cls");
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);
 
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    CloseHandle(semaphore);
    CloseHandle(pipe);

	return 0;
}

void createProcess(TCHAR* path, PROCESS_INFORMATION& processInfo)
{
    STARTUPINFO startupInfo;

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory(&processInfo, sizeof(processInfo));

    if (!CreateProcess(NULL,
        path,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &startupInfo,
        &processInfo)
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }
}


