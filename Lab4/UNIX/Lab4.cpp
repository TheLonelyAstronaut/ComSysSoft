#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string>
#include <sstream>

void parentProcess(char* path);
void* childProcess(void* params);
int getchBundle();
std::vector<pthread_t> threadInfoVector;
std::vector<bool> canCloseVector;
pthread_mutex_t mutex;

int _kbhit() {
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
 
int _getch()
{
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}

int main(int argc, char** argv)
{
	parentProcess(argv[0]);
	return 0;
}

int getchBundle()
{
	if (_kbhit()) return _getch();
	else return -1;
}

void parentProcess(char* path)
{
	char symbol = 0;
	int currentProcess = 0;
	bool wasProcessKilled = false;
    pthread_mutex_init(&mutex, NULL);

	while (1)
	{
		usleep(1000);
		symbol = getchBundle();

		switch (symbol)
		{
		case '+':
		{
			int currentThreadId = threadInfoVector.size() + 1;

            pthread_t id; 
            pthread_create(&id, NULL, &childProcess, &currentThreadId);
			threadInfoVector.push_back(id);
			canCloseVector.push_back(false);

			break;
		}
		case '-':
		{
			if (!threadInfoVector.size()) return;
			
			for (int i = canCloseVector.size(); i != 0; --i) {
				
				if (!canCloseVector[i - 1]) {
					canCloseVector[i - 1] = true;
					
                    pthread_join(threadInfoVector[i-1], NULL);
					canCloseVector.pop_back();
					threadInfoVector.pop_back();

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
			
			canCloseVector.clear();
			threadInfoVector.clear();

			break;
		}
		default:
			break;
		}
	}
}


void* childProcess(void* params)
{
	char buffer[40];
	int currentThreadId = *static_cast<const int*>(params);
    sprintf(buffer, "%d_Thread", currentThreadId);
	bool exit = false;
	if (currentThreadId == 1) std::cout << std::endl;
    
	while (!exit)
	{
        pthread_mutex_lock(&mutex);

		if (canCloseVector[currentThreadId - 1]) {
			pthread_mutex_unlock(&mutex);
			break;
		}

		for (int i = 0; i < strlen(buffer); ++i)
		{
			if (currentThreadId <= canCloseVector.size() && canCloseVector[currentThreadId - 1]) {
				exit = true;
				break;
			}

			printf("%c", buffer[i]);
			usleep(100000);
		}

		std::cout << std::endl;

        pthread_mutex_unlock(&mutex);
		usleep(100000);
	}

    pthread_exit(NULL);
	
}