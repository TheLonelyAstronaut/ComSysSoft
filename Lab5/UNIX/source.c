#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <aio.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#define FILE_SIZE 3

void* readFromFile(void* );
void* writeToFile(void* );

int (*AsyncRead) (struct ReaderWriter *);
int (*AsyncWrite) (struct ReaderWriter *);
const char *files[FILE_SIZE] = { "inp1.txt","inp2.txt", "inp3.txt" };
void *dynamicLibrary;

pthread_mutex_t mutex;

struct ReaderWriter
{
  int readFileDescriptor;
  int writeFileDescriptor;
  char buffer[100];
  size_t byteNumber;
  off_t outPosition;
  struct aiocb aiocbVariable;
} asyncObject;

int main()
{
  pthread_mutex_init(&mutex, NULL);
  
  dynamicLibrary = dlopen("./AsyncOperations.so",RTLD_NOW);
  
  AsyncRead =(int(*)(struct ReaderWriter*)) dlsym(dynamicLibrary,"AsyncRead");
  AsyncWrite = (int(*)(struct ReaderWriter*))dlsym(dynamicLibrary,"AsyncWrite");  
  
  pthread_t readerThread, writerThread;
  
  asyncObject.aiocbVariable.aio_offset = 0;
  asyncObject.outPosition = 0;
  asyncObject.aiocbVariable.aio_buf = asyncObject.buffer;
  asyncObject.byteNumber = sizeof(asyncObject.buffer);
  asyncObject.aiocbVariable.aio_sigevent.sigev_notify = SIGEV_NONE; 

  
  pthread_create(&readerThread, NULL, readFromFile, NULL);
  pthread_create(&writerThread, NULL, writeToFile, NULL);
  
  pthread_join(readerThread, NULL);
  pthread_join(writerThread, NULL);

  
  pthread_mutex_destroy(&mutex);
  return 0;
}

void* readFromFile(void* data)
{
  int ReadFile;

  for(int i = 0; i < FILE_SIZE; i++)
  {
    pthread_mutex_lock(&mutex);
    ReadFile = open(files[i], O_RDONLY);
    asyncObject.readFileDescriptor = ReadFile;
    AsyncRead(&asyncObject);
    close(ReadFile);
    pthread_mutex_unlock(&mutex);
    usleep(100);
  }
  return NULL;
}

void* writeToFile(void* data)
{
  int WriteFile;
  
  int i = 0;

  WriteFile = open("out.txt", O_WRONLY | O_CREAT | O_APPEND |O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  
  while(i < FILE_SIZE)
  {   
    pthread_mutex_lock(&mutex); 
    asyncObject.writeFileDescriptor = WriteFile;
    AsyncWrite(&asyncObject);
    pthread_mutex_unlock(&mutex);
    usleep(100);
    i++;
  }

  close(WriteFile);
  return NULL; 
}