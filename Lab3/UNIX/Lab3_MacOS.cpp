#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <termios.h>
#include <algorithm>
#include <ncurses.h>

#define SEM_ID 228

int createSemaphore();
void doChildWork();
void doParentWork();
struct sembuf setSemaphoreValue(int num,int op,int flag);
int fdescriptors[2];       
void replaceLastN(char buff[256]);


int main(){
  bool exit=false;
  int semaphoreSetID;

  semaphoreSetID = createSemaphore();
  if(semaphoreSetID == -1) return 0;

  if (pipe(fdescriptors)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 

  int procPID = fork();
  switch(procPID) {
    case -1:
      perror("fork");
    return -1;

    case 0:
      doChildWork();
    return 13;

    default: {
      doParentWork();
    break;
    }
    }
  int status;
  waitpid(procPID, &status, 0);
  semctl(semaphoreSetID, 0, IPC_RMID);   //deleting our set of semaphores
  return 0;
}

int createSemaphore(){
  int sem_set_id;
  int status;
  union semun sem_val;

  sem_set_id = semget(SEM_ID, 1, IPC_CREAT);
  if (sem_set_id == -1) {
    perror("Something went wrong ");
    return -1;
  }

  sem_val.val=0;
  status=semctl(sem_set_id,0,SETVAL,sem_val);

  return sem_set_id;
}

void doChildWork(){
  struct sembuf decrement, increment, waitStatus;
  bool exit=false;
  char buff[256];

  decrement = setSemaphoreValue(0,-1,0);
  increment = setSemaphoreValue(0,1,0);
  waitStatus = setSemaphoreValue(0,0,0);

  printf("Type 'exit' to terminate\n");

  while(!exit){

    close(fdescriptors[0]);
    semop(SEM_ID, &waitStatus, 1);
    semop(SEM_ID, &increment, 1);

    printf("Enter your string > ");
    fgets(buff, 256, stdin);
    replaceLastN(buff);
    write(fdescriptors[1], buff, strlen(buff)+1);
    usleep(10000);

    if(!strcmp(buff, "exit")) {
			exit = true;
      semop(SEM_ID, &decrement, 1);
			break;
		}

    semop(SEM_ID, &decrement, 1);
    }
}

void replaceLastN(char buff[256]){
  for(int i=0;i<256;i++){
      if(buff[i]=='\n'){
        buff[i]='\0';
      }
    }
}

void doParentWork(){
  struct sembuf decrement, increment, waitStatus;
  bool exit=false;
  char buff[256];

  decrement = setSemaphoreValue(0,-1,0);
  increment = setSemaphoreValue(0,1,0);
  waitStatus = setSemaphoreValue(0,0,0);

  while(!exit){

    close(fdescriptors[1]);
    semop(SEM_ID, &waitStatus, 1);
    semop(SEM_ID, &increment, 1);

    read(fdescriptors[0], buff, 256);
    printf("Received: <%s>\n", buff);

    if(!strcmp(buff, "exit")) {
			exit = true;
      semop(SEM_ID, &decrement, 1);
			break;
		}

    semop(SEM_ID, &decrement, 1);
    }
}


struct sembuf setSemaphoreValue(int num,int op,int flag){
  struct sembuf sem_action;
  sem_action.sem_num = num;
  sem_action.sem_op = op;
  sem_action.sem_flg = flag;
  return sem_action;
}