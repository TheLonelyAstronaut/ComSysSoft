#include "pch.h"

#ifdef __cplusplus 
extern "C" {
#endif

	struct ReaderWriter
	{
		HANDLE readFileDescriptor;
		HANDLE writeFileDescriptor;
		DWORD byteNumber;
		CHAR buffer[200];
		DWORD outPosition;
		OVERLAPPED overlapped;
	};

	__declspec(dllexport) BOOL AsyncRead(ReaderWriter* data)
	{
		BOOL readResult;
		DWORD NumberOfReadBytes;
		data->overlapped.Offset = 0;

		ReadFile(data->readFileDescriptor, data->buffer, data->byteNumber, NULL, &data->overlapped);

		WaitForSingleObject(data->overlapped.hEvent, INFINITE);

		readResult = GetOverlappedResult(data->readFileDescriptor, &data->overlapped, &NumberOfReadBytes, TRUE);
		if (readResult) {
			data->byteNumber = NumberOfReadBytes;
			data->buffer[data->byteNumber] = 0;
		}
		else data->byteNumber = 0;

		return readResult;
	}

	__declspec(dllexport) BOOL AsyncWrite(ReaderWriter* data)
	{
		BOOL writeResult;
		DWORD NumberOfWriteBytes;
		data->overlapped.Offset = data->outPosition;

		WriteFile(data->writeFileDescriptor, data->buffer, data->byteNumber, NULL, &data->overlapped);
		WaitForSingleObject(data->overlapped.hEvent, INFINITE);

		writeResult = GetOverlappedResult(data->writeFileDescriptor, &data->overlapped, &NumberOfWriteBytes, TRUE);
		if (writeResult) {
			data->outPosition = data->outPosition + NumberOfWriteBytes;
			data->byteNumber = sizeof(data->buffer);
		}

		return writeResult;
	}

#ifdef __cplusplus
}
#endif