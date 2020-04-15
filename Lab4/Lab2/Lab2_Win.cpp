#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <vector>
#include <wchar.h>
#include <tchar.h>
#include <processthreadsapi.h>

void parentProcess(TCHAR* path);
void childProcess(int processID);
int getchBundle();
PROCESS_INFORMATION createNewProcess(TCHAR* path, TCHAR* processAddress);

int _tmain(int argc, TCHAR** argv)
{
	if (argc == 2) childProcess(_ttoi(argv[1]));
	else parentProcess(argv[0]);
	return 0;
}

int getchBundle()
{
	if (_kbhit()) return _getch();
	else return -1;
}

void parentProcess(TCHAR* path)
{
	char symbol = 0;
	int currentProcess = 0;
	bool wasProcessKilled = false;
	TCHAR processAddress[40];
	memset(processAddress, 40, 0);
	std::vector<HANDLE> closeEventVector;
	std::vector<HANDLE> canPrintEventVector;
	std::vector<PROCESS_INFORMATION> processInfoVector;


	while (1)
	{
		Sleep(1);
		symbol = getchBundle();

		switch (symbol)
		{
		case '+':
		{
			//std::cout << "here" << std::endl;
			swprintf(processAddress, 40, L"%d", closeEventVector.size()+1);
			closeEventVector.push_back(CreateEvent(NULL, FALSE, FALSE, processAddress));

			swprintf(processAddress, 40, L"%dp", canPrintEventVector.size()+1);
			canPrintEventVector.push_back(CreateEvent(NULL, TRUE, FALSE, processAddress));
			//std::cout << canPrintEventVector.back() << std::endl;

			processInfoVector.push_back(createNewProcess(path, processAddress));
			break;
		}
		case '-':
		{
			if (!closeEventVector.size()) break;

			SetEvent(closeEventVector.back());
			WaitForSingleObject(processInfoVector[closeEventVector.size() - 1].hProcess, 0);

			if (!CloseHandle(closeEventVector.back()) || !CloseHandle(canPrintEventVector.back())) std::cout << "Closing handle event failed: " << GetLastError();
			//if(!TerminateProcess(processInfoVector.back().hThread, 0)) std::cout << "Terminate process failed: " << GetLastError();

			//canPrintEventVector.back().hProcess;
			closeEventVector.pop_back();
			canPrintEventVector.pop_back();
			//processInfoVector.pop_back();

			if (currentProcess >= closeEventVector.size())
			{
				currentProcess = 0;
				wasProcessKilled = true;
				//std::cout << std::endl;
			}

			break;
		}
		case 'q':
		{
			if (!closeEventVector.size()) return;

			while (closeEventVector.size())
			{
				SetEvent(closeEventVector.back());
				WaitForSingleObject(processInfoVector[closeEventVector.size() - 1].hProcess, 0);
				//std::cout << "Size: " << closeEventVector.size() << std::endl;

				if (!CloseHandle(closeEventVector.back()) || !CloseHandle(canPrintEventVector.back())) std::cout << "Closing handle event failed: " << GetLastError();
				//if(!CloseHandle(processInfoVector.back().hThread) || !CloseHandle(processInfoVector.back().hProcess)) std::cout << "Closing handle event failed: " << GetLastError();
				
				closeEventVector.pop_back();
				canPrintEventVector.pop_back();
				processInfoVector.pop_back();
			}

			wasProcessKilled = true;
			currentProcess = 0;
			break;
		}
		default:
			break;
		}

		//std::cout << "After all" << std::endl;

		if (canPrintEventVector.size() && WaitForSingleObject(canPrintEventVector[currentProcess], 0) == WAIT_TIMEOUT)
		{
			if (currentProcess >= canPrintEventVector.size() - 1) 
			{
				std::cout << std::endl;
				currentProcess = 0;
			}
			else if (!wasProcessKilled) currentProcess++;
			wasProcessKilled = false;

			SetEvent(canPrintEventVector[currentProcess]);
		}
	}
}

PROCESS_INFORMATION createNewProcess(TCHAR* path, TCHAR* processAddress)
{
	WCHAR args[100];
	swprintf(args, 100, L"%s %s", path, processAddress);

	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(processInfo));

	if (!CreateProcess(path, args, NULL, NULL, FALSE, NULL, NULL, NULL, &startupInfo, &processInfo))
		std::cout << "Process creation failed: " << GetLastError() << std::endl;

	return processInfo;
}


void childProcess(int processID)
{
	TCHAR buffer[40];
	HANDLE closeEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, _itow(processID, buffer, 40));
	//wprintf(buffer);
	swprintf(buffer, 40, L"%dp", processID);
	//wprintf(buffer);
	HANDLE canPrintEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, buffer);

	while (1)
	{
		//std::cout << processID << std::endl;
		Sleep(1000);
		//std::cout << (WaitForSingleObject(canPrintEvent, INFINITE) == WAIT_OBJECT_0) << " " << processID << std::endl;
		if (WaitForSingleObject(canPrintEvent, INFINITE) == WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(closeEvent, 0) == WAIT_OBJECT_0)
			{
				CloseHandle(closeEvent);
				CloseHandle(canPrintEvent);
				return;
			}

			std::cout << processID << " ";
			ResetEvent(canPrintEvent);
			//ResetEvent(closeEvent);
		}

		if (WaitForSingleObject(closeEvent, 0) == WAIT_OBJECT_0)
		{
			CloseHandle(closeEvent);
			CloseHandle(canPrintEvent);
			return;
		}
	}
}

