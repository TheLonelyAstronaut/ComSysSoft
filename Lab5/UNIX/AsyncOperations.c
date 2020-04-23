#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

struct ReaderWriter
{
  int readFileDescriptor;
  int writeFileDescriptor;
  char buffer[100];
  size_t byteNumber;
  off_t outPosition;
  struct aiocb aiocbVariable;
};

void AsyncWrite(struct ReaderWriter *object)
{
  int numberWriteBytes;
  object->aiocbVariable.aio_offset = object->outPosition;
  object->aiocbVariable.aio_fildes = object->writeFileDescriptor;
  object->aiocbVariable.aio_nbytes = object->byteNumber;

  aio_write(&object->aiocbVariable);
  
  while(aio_error(&object->aiocbVariable) == EINPROGRESS);
  usleep(100);

  object->outPosition = object->outPosition + aio_return(&object->aiocbVariable);
  object->byteNumber = sizeof(object->buffer);
  return;
}

void AsyncRead(struct ReaderWriter *object)
{
  int numberOfReadBytes;
  object->aiocbVariable.aio_offset = 0;
  object->aiocbVariable.aio_fildes = object->readFileDescriptor;
  object->aiocbVariable.aio_nbytes = object->byteNumber;

  aio_read(&object->aiocbVariable);
  
  while(aio_error(&object->aiocbVariable) == EINPROGRESS){}
  usleep(100);
  
  object->byteNumber = aio_return(&object->aiocbVariable);
  object->buffer[object->byteNumber] = 0;
  return;
}