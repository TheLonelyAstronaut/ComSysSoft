#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <sys/mman.h>

#define BLOCK_INFO_SIZE sizeof(struct blockInfo)
#define SWAP_SIZE 1048576
#define CHUNK_SIZE 4096


bool isAllocatorInit = false, isSwapFileInit = false;
int fileDescriptor;
void *startMemoryAddressPointer;
void *lastMemoryAdressPointer;
void *startFileAddressPointer;
void *lastFileAddressPointer;


struct blockInfo
{
  int isMemoryFree;
  size_t blockSize;
};

void initAllocator()
{
  //request last adress of free memory from system
  lastMemoryAdressPointer = sbrk(0);
  startMemoryAddressPointer = lastMemoryAdressPointer;
  isAllocatorInit = true;
}

void initSwapFile()
{
  fileDescriptor = open("swap", O_RDWR | O_CREAT | O_TRUNC);
  lseek(fileDescriptor, SWAP_SIZE-1, SEEK_SET);
  write(fileDescriptor, "", 1);
  lseek(fileDescriptor, 0, SEEK_SET);

  lastFileAddressPointer = mmap(0, SWAP_SIZE, PROT_WRITE | PROT_READ , MAP_SHARED , fileDescriptor, 0);
  printf("%d\n", lastFileAddressPointer);
  startFileAddressPointer = lastFileAddressPointer;
  isSwapFileInit = true;
}

void* locateByPointers(void* startPointer, void* lastPointer, size_t size)
{
  void *currentMemoryPointer;
  struct blockInfo *currentMemoryBlock;
  void *resultLocation = 0;

  currentMemoryPointer = startPointer; //start search from the beginning of the availablememory

  for (; currentMemoryPointer != lastPointer; currentMemoryPointer = (void*)((char*)currentMemoryPointer + currentMemoryBlock->blockSize))
  {
    currentMemoryBlock = (struct blockInfo *)currentMemoryPointer;
    if (currentMemoryBlock->isMemoryFree && currentMemoryBlock->blockSize >= size)
    {
      currentMemoryBlock->isMemoryFree = 0;
      resultLocation = currentMemoryPointer;
      break;
    }
  }

  //cant find free memory right now,request memory from system
  if (!resultLocation)
  {
    resultLocation = lastPointer;
    lastPointer = (void*)((char*)lastPointer + size);
    currentMemoryBlock = (struct blockInfo *)resultLocation;
    currentMemoryBlock->isMemoryFree = 0;
    currentMemoryBlock->blockSize = size; 
  }

  resultLocation = (void*)((char*)resultLocation + BLOCK_INFO_SIZE);

  return resultLocation;
}

void* customMalloc(size_t size, bool useSwap = false)
{
  size += BLOCK_INFO_SIZE;

  if(useSwap || (*(int*)sbrk(2*CHUNK_SIZE) == -1 && *(int*)sbrk(CHUNK_SIZE) != -1))
  {
    if(!isSwapFileInit) initSwapFile();
    return locateByPointers(startFileAddressPointer, lastFileAddressPointer, size);
  }
  else
  {
    if(!isAllocatorInit) initAllocator();
    return locateByPointers(startMemoryAddressPointer, lastMemoryAdressPointer, size);
  }
}

void customFree(void *blockPointer)
{
  struct blockInfo *tempBlock;
  tempBlock = (struct blockInfo *)((char*)blockPointer - BLOCK_INFO_SIZE);
  tempBlock->isMemoryFree = 1;
  return;
}