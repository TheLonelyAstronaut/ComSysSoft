#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initAllocator();
void initSwapFile();
void* customMalloc(size_t size, bool useSwap = false);
void customFree(void *blockPointer);
void* locateByPointers(void* startPointer, void* lastPointer, size_t size);

int main(){
  //initSwapFile();
  char *str;

  str = (char *) customMalloc(15, true);
  strcpy(str, "Here's new malloc");
  printf("String = %s\n", str);

  int* intTest = (int*)customMalloc(sizeof(int));
  *(intTest) = 23;
  printf("intTest after malloc:%d\n", *intTest);

  double* doubleTest = (double*)customMalloc(sizeof(double)*2, true);
  *(doubleTest) = 23.41;
  *(doubleTest + 1) = 221.25;
  printf("doubleTest array (swap file) after malloc: %.2f, %.2f\n", *(doubleTest), *(doubleTest + 1));

  customFree(str);
  customFree(intTest);
  customFree(doubleTest);

  doubleTest = (double*)customMalloc(sizeof(double)*2, true);
  *(doubleTest) = 2.12;
  *(doubleTest + 1) = 21.25;
  printf("doubleTest array (swap file) after second malloc: %.2f, %.2f\n", *(doubleTest), *(doubleTest + 1));

  customFree(doubleTest);

  return 0;
}



