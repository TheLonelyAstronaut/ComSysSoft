#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <Windows.h>
#include <conio.h>
#include <iostream>
#include <stdio.h>
using namespace std;

#define FSSIZE					1114112
#define BLOCKSIZE				256
#define MFTSIZE					65536
#define BLOCKSCOUNT				(FSSIZE - MFTSIZE) / BLOCKSIZE

#define MAXFILECOUNT			1000
#define MAXNAME					100
#define MAXTYPE					10
#define FOLDERFILESMAXNUMBER	25

#define FSMAINFILE				"D:\\File.bin"
#define FOLDER					"FOLDER"
#define FILE					"FILE"

struct FileTable
{
	size_t freeSpace = FSSIZE - MFTSIZE;
	size_t freeBlocksOffsets[BLOCKSCOUNT];
	size_t filesOffsets[MAXFILECOUNT];

	FileTable()
	{
		for (int i = 0; i < BLOCKSCOUNT; i++) freeBlocksOffsets[i] = BLOCKSIZE * i + MFTSIZE;
		for (int g = 0; g < MAXFILECOUNT; g++)  filesOffsets[g] = 0;
	}
};

struct Block
{
	char data[BLOCKSIZE];

	Block()
	{
		for (int i = 0; i < BLOCKSIZE; i++) data[i] = 0;
	}
};

struct Entity
{
	char name[MAXNAME];
	char type[MAXTYPE];
	size_t blocks[FOLDERFILESMAXNUMBER];

	Entity()
	{
		for (int i = 0; i < MAXNAME; i++) name[i] = 0;

		for (int g = 0; g < MAXTYPE; g++) type[g] = 0;

		for (int m = 0; m < FOLDERFILESMAXNUMBER; m++) blocks[m] = 0;

	}
};