#pragma once
#include "DataStructers.h"

class FSEmulator
{
	FileTable fileTable;
	string pwd;
	fstream FSDrive;
	size_t offset;
	HANDLE console;
	vector<string> splitFolderName(string Name);

	void deleteFolder(Entity folder);
	void deleteInstace(size_t offset);

	void saveFileTable();
	void saveInstance(size_t offset, Entity block);
	void saveBlock(size_t offset, Block block);
	void saveFile(Entity file, size_t offsetToFile);

	void editFile(size_t offsetToFile);

	Entity getFolder(string folderName);
	size_t getFolderOffset(string folderName);
	size_t getFreeBlockOffset();
	size_t getSubFolderOffset(vector<string> path, Entity baseFolder, int index);

	int keyHandler();

	Entity readEntity(size_t offset);
	Block readBlock(size_t offset);

	int trim(char *str);

public:

	FSEmulator();
	~FSEmulator();

	void openFile(string fileName);

	void dir();
	void showFolder(Entity folder, int lvl = 0);

	void createEntity(string type,string fileName);

	size_t addInstanceToFolder(Entity entity);
	size_t addInstanceToMFT(Entity entity);

	void remove(string fileName);
	void cut(string fileName);
	void paste();

	
	void changeFolder(string newFolder);
	string getCurrentFolder();

	void format();
};