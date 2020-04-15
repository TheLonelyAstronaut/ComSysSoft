#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <vector>
#include <wchar.h>
#include <tchar.h>
#include <processthreadsapi.h>
#include <aclapi.h>

void parentProcess(TCHAR* path);
DWORD WINAPI childProcess(CONST LPVOID lpParam);
int getchBundle();
std::vector<HANDLE> threadInfoVector;
std::vector<bool> canCloseVector;
CRITICAL_SECTION critical;


int _tmain(int argc, TCHAR** argv)
{
	parentProcess(argv[0]);
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

	InitializeCriticalSection(&critical);


	while (1)
	{
		Sleep(1);
		symbol = getchBundle();

		switch (symbol)
		{
		case '+':
		{
			int currentThreadId = threadInfoVector.size() + 1;

			threadInfoVector.push_back(CreateThread(NULL, 0, &childProcess, &currentThreadId, 0, NULL));
			canCloseVector.push_back(false);

			break;
		}
		case '-':
		{
			if (!threadInfoVector.size()) return;
			
			for (int i = canCloseVector.size(); i != 0; --i) {
				
				if (!canCloseVector[i - 1]) {
					canCloseVector[i - 1] = true;
					
					if (WaitForSingleObject(threadInfoVector[i - 1], INFINITE) == WAIT_OBJECT_0) {
						canCloseVector.pop_back();
						threadInfoVector.pop_back();
					}

					break;
				}
			}
			
			break;
		}
		case 'q':
		{
			if (!threadInfoVector.size()) return;

			for (int i = canCloseVector.size(); i != 0; --i) {
				canCloseVector[i - 1] = true;
			}
			
			if (WaitForMultipleObjects(threadInfoVector.size(), &threadInfoVector[0], TRUE, INFINITE) == WAIT_OBJECT_0) {
				canCloseVector.clear();
				threadInfoVector.clear();
			}

			break;
		}
		default:
			break;
		}
	}
}


DWORD WINAPI childProcess(CONST LPVOID lpParam)
{
	TCHAR buffer[40];
	int currentThreadId = *static_cast<const int*>(lpParam);
	swprintf(buffer, 40, L"%d_Process", currentThreadId);
	bool exit = false;
	if (currentThreadId == 1) std::cout << std::endl;

	while (!exit)
	{
		EnterCriticalSection(&critical);

		if (canCloseVector[currentThreadId - 1]) {
			LeaveCriticalSection(&critical);
			break;
		}

		for (int i = 0; i < _tcslen(buffer); ++i)
		{
			if (currentThreadId <= canCloseVector.size() && canCloseVector[currentThreadId - 1]) {
				exit = true;
				break;
			}

			wprintf(L"%c", buffer[i]);
			Sleep(100);
		}

		std::cout << std::endl;

		LeaveCriticalSection(&critical);
		Sleep(100);
	}

	ExitThread(0);
	
}

