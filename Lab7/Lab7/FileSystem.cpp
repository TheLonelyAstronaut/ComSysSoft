#include "FSEmulator.h"
using namespace std;

FSEmulator::FSEmulator()
{
	pwd = "";
	offset = 0;
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	FSDrive.open("FS.out", ios::in | ios::out | ios::binary);
	FSDrive.read((char*)&fileTable, sizeof(FileTable));
}

FSEmulator::~FSEmulator()
{
	FSDrive.close();
}

void FSEmulator::dir()
{
	Entity temp;
	if (pwd == "")
	{
		for (int i = 0; i < MAXFILECOUNT; i++) {
			if (fileTable.filesOffsets[i] != 0)
			{
				temp = readEntity(fileTable.filesOffsets[i]);
				if (strcmp(temp.type, ((string)FOLDER).c_str()) == 0)
				{
					cout << " <" << temp.type << ">";
					cout << " " << temp.name << endl;
					showFolder(temp,0);
				}
				else {
					cout << " " << temp.name << endl;
				}
			}
		}
	}
	else
	{
		temp = getFolder(pwd);
		cout << temp.name << endl;
		showFolder(temp,0);
	}
	cout << endl;
}


void FSEmulator::createEntity(string type,string fileName)
{
	Entity file;
	strcpy_s(file.type, type.c_str());
	strcpy_s(file.name, fileName.c_str());

	if (pwd == "")
	{
		if (addInstanceToMFT(file) == 0)
			cout << " Not enough free space" << endl;
	}
	else {
		if (addInstanceToFolder(file) == 0)
			cout << " Folder is full" << endl;
	}
		
}

void FSEmulator::remove(string fileName)
{
	Entity temp;
	if (pwd == "")
	{
		for (int i = 0; i < MAXFILECOUNT; i++) {
			if (fileTable.filesOffsets[i] != 0)
			{
				temp = readEntity(fileTable.filesOffsets[i]);
				if (strcmp(temp.name, fileName.c_str()) != 0) continue;
				if (strcmp(temp.type, ((string)FOLDER).c_str()) == 0) {
					deleteFolder(temp);
				}
				deleteInstace(fileTable.filesOffsets[i]);
				fileTable.filesOffsets[i] = 0;
				saveFileTable();
				return;
			}
		}
	}
	else
	{
		Entity some;
		temp = getFolder(pwd);
		for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
			if (temp.blocks[i] != 0)
			{
				some = readEntity(temp.blocks[i]);
				if (strcmp(some.name, fileName.c_str()) != 0) continue;
				if (strcmp(some.type, ((string)FOLDER).c_str()) == 0) {
					deleteFolder(some);
				}
				deleteInstace(temp.blocks[i]);
				temp.blocks[i] = 0;
				saveInstance(getFolderOffset(pwd), temp);
				return;
			}
	}
	cout << "Entity doesn't exist" << endl;
}

void FSEmulator::deleteFolder(Entity folder)
{
	for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
	{
		if (folder.blocks[i] != 0)
		{
			Entity temp = readEntity(folder.blocks[i]);
			if (strcmp(temp.type, ((string)FOLDER).c_str()) == 0) {
				deleteFolder(temp);
			}
			deleteInstace(folder.blocks[i]);
		}
	}
}

void FSEmulator::deleteInstace(size_t offset)
{
	fileTable.freeBlocksOffsets[(offset - MFTSIZE) / BLOCKSIZE] = offset;
	fileTable.freeSpace += BLOCKSIZE;
	FSDrive.seekp(offset);
	FSDrive.write((char*)&Block(), sizeof(Entity));
	FSDrive.flush();
}


size_t FSEmulator::addInstanceToMFT(Entity entity)
{
	for (int i = 0; i < MAXFILECOUNT; i++)
		if (fileTable.filesOffsets[i] == 0)
		{
			//we find available block
			for (int j = 0; j < BLOCKSCOUNT; j++) {
				if (fileTable.freeBlocksOffsets[j] != 0)
				{
					fileTable.filesOffsets[i] = fileTable.freeBlocksOffsets[j];
					fileTable.freeBlocksOffsets[j] = 0;
					fileTable.freeSpace -= BLOCKSIZE;
					saveInstance(fileTable.filesOffsets[i], entity);
					return fileTable.filesOffsets[i];
				}
			}
			break;
		}
	return 0;
}


size_t FSEmulator::addInstanceToFolder(Entity entity)
{
	Entity folder = getFolder(pwd);
	for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
		if (folder.blocks[i] == 0)
		{
			//we find available block
			for (int j = 0; j < BLOCKSCOUNT; j++) {
				if (fileTable.freeBlocksOffsets[j] != 0)
				{
					folder.blocks[i] = fileTable.freeBlocksOffsets[j];
					fileTable.freeBlocksOffsets[j] = 0;
					fileTable.freeSpace -= BLOCKSIZE;
					saveInstance(folder.blocks[i], entity);
					saveInstance(getFolderOffset(pwd), folder);
					return folder.blocks[i];
				}
			}
		}
	return 0;
}

void FSEmulator::cut(string fileName)
{
	Entity temp;
	if (pwd == "")
	{
		for (int i = 0; i < MAXFILECOUNT; i++)
		{
			if (fileTable.filesOffsets[i] != 0)
			{
				temp = readEntity(fileTable.filesOffsets[i]);
				if (strcmp(temp.name, fileName.c_str()) != 0) continue;
				offset = fileTable.filesOffsets[i];
				fileTable.filesOffsets[i] = 0;
				return;
			}
		}
	}
	else
	{
		Entity folder;
		folder = getFolder(pwd);
		for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
			if (folder.blocks[i] != 0)
			{
				temp = readEntity(folder.blocks[i]);
				if (strcmp(temp.name, fileName.c_str()) != 0) continue;
				offset = folder.blocks[i];
				folder.blocks[i] = 0;
				saveInstance(getFolderOffset(pwd), folder);
				return;
			}
	}
	cout << " Entity doesn't exist" << endl;
}

void FSEmulator::paste()
{
	if (offset == 0) return;
	if (pwd == "")
	{
		for (int i = 0; i < MAXFILECOUNT; i++)
			if (fileTable.filesOffsets[i] == 0) {
				fileTable.filesOffsets[i] = offset;
				break;
			}
	}
	else
	{
		Entity folder;
		folder = getFolder(pwd);
		for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
			if (folder.blocks[i] == 0)
			{
				folder.blocks[i] = offset;
				saveInstance(getFolderOffset(pwd), folder);
				break;
			}
	}
	offset = 0;
}

void FSEmulator::openFile(string fileName)
{
	string folderName;
	Entity temp;
	if (pwd == "")
	{
		for (int i = 0; i < MAXFILECOUNT; i++) {
			if (fileTable.filesOffsets[i] != 0)
			{
				temp = readEntity(fileTable.filesOffsets[i]);
				if (strcmp(temp.name, fileName.c_str()) != 0) continue;
				if (strcmp(temp.type, ((string)FOLDER).c_str()) == 0) {
					cout << "Can't edit folder" << endl;
					return;
				}
				editFile(fileTable.filesOffsets[i]);
				return;
			}
		}
	}
	else
	{
		Entity folder = getFolder(pwd);
		for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
			if (folder.blocks[i] != 0)
			{
				temp = readEntity(folder.blocks[i]);
				if (strcmp(temp.name, fileName.c_str()) != 0) continue;
				if (strcmp(temp.type, ((string)FOLDER).c_str()) == 0) {
					cout << "Can't edit folder" << endl;
					return;
				}
				editFile(folder.blocks[i]);
				return;
			}
	}
	cout << " File doesn't exist" << endl;
}

void FSEmulator::editFile(size_t offsetToFile)
{
	Entity file = readEntity(offsetToFile);
	Block temp;
	char tempText[BLOCKSIZE];
	string text = "";
	for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
		if (file.blocks[i] != 0)
		{
			temp = readBlock(file.blocks[i]);
			strcpy_s(tempText, temp.data);
			text += string(tempText);
		}
	system("cls");
	cout << text;
	SetConsoleCursorPosition(console, COORD{ 0, 0 });
	while (true)
	{
		if (_kbhit())
		{
			if (!keyHandler()) break;
		}
	}
	saveFile(file, offsetToFile);
	system("cls");
}

void FSEmulator::saveFile(Entity file, size_t offsetToFile)
{
	COORD size, pos;
	size_t newOffset;
	PSMALL_RECT rect = (PSMALL_RECT)malloc(sizeof(PSMALL_RECT));
	rect->Left = 0; rect->Top = 0; rect->Bottom = 80; rect->Right = 79;
	pos.X = 0; pos.Y = 0; size.X = 1; size.Y = 1;
	//screen buffer
	PCHAR_INFO buff = (PCHAR_INFO)calloc(1, sizeof(PCHAR_INFO));
	Block newBlock;
	int blockNumber = 0, blockIndex = 0;
	while (true)
	{
		if (ReadConsoleOutputA(console, buff, size, pos, rect) == 0)
			break;
		newBlock.data[blockIndex++] = buff[0].Char.AsciiChar;
		if (blockIndex == BLOCKSIZE - 1)
		{
			newBlock.data[blockIndex++] = '\0';
			if (blockNumber == 24) break;
			blockIndex = 0;
			newOffset = file.blocks[blockNumber] == 0 ? getFreeBlockOffset() : file.blocks[blockNumber];
			file.blocks[blockNumber++] = newOffset;
			saveBlock(newOffset, newBlock);
			newBlock = Block();
		}
		rect->Bottom = 80; rect->Right = 79;
		//go to next line
		if (++rect->Left == 80)
		{
			blockIndex = trim(newBlock.data);
			rect->Left = 0;
			++rect->Top;
		}
	}
	//save our data block
	if (trim(newBlock.data) != 0)
	{
		newOffset = file.blocks[blockNumber] == 0 ? getFreeBlockOffset() : file.blocks[blockNumber];
		file.blocks[blockNumber++] = newOffset;
		saveBlock(newOffset, newBlock);
	}
	//reset extra data
	for (int i = blockNumber; i < FOLDERFILESMAXNUMBER; i++)
		if (file.blocks[i] != 0)
		{
			fileTable.freeBlocksOffsets[(file.blocks[i] - MFTSIZE) / BLOCKSIZE] = file.blocks[i];
			fileTable.freeSpace += BLOCKSIZE;
			file.blocks[i] = 0;
		}
	saveInstance(offsetToFile, file);
}

int FSEmulator::trim(char *str)
{
	int i = BLOCKSIZE - 1;
	while ((str[i] == ' ' || str[i] == '\0' || str[i] == '\n') && i >= 0) i--;
	if (i == -1) return 0;
	str[i + 1] = '\n';
	return i + 2;
}

void FSEmulator::saveFileTable()
{
	FSDrive.seekp(0);
	FSDrive.write(reinterpret_cast<char*>(&fileTable), sizeof(fileTable));
	FSDrive.flush();
}

void FSEmulator::saveInstance(size_t offset, Entity block)
{
	saveFileTable();
	FSDrive.seekp(offset);
	FSDrive.write((char*)&block, sizeof(Entity));
	std::cout << FSDrive.good() << std::endl;
	FSDrive.flush();
}

void FSEmulator::saveBlock(size_t offset, Block block)
{
	FSDrive.seekp(offset);
	FSDrive.write((char*)&block, sizeof(Block));
	FSDrive.flush();
}

Entity FSEmulator::readEntity(size_t offset)
{
	Entity file;
	FSDrive.seekg(offset);
	FSDrive.read((char*)&file, sizeof(Entity));
	return file;
}

Block FSEmulator::readBlock(size_t offset)
{
	Block block;
	FSDrive.seekg(offset);
	FSDrive.read((char*)&block, sizeof(Block));
	return block;
}

int FSEmulator::keyHandler()
{
	int word;
	COORD pos;
	PCONSOLE_SCREEN_BUFFER_INFO  currentPos = (PCONSOLE_SCREEN_BUFFER_INFO)malloc(sizeof(PCONSOLE_SCREEN_BUFFER_INFO));
	word = _getch();
	if (word == 8)	//backspace
	{
		GetConsoleScreenBufferInfo(console, currentPos);
		pos.X = currentPos->dwCursorPosition.X == 0 ? currentPos->dwCursorPosition.X : currentPos->dwCursorPosition.X - 1;
		pos.Y = currentPos->dwCursorPosition.Y;
		SetConsoleCursorPosition(console, pos);
		cout << " ";
		SetConsoleCursorPosition(console, pos);
	}
	else if (word == 224)	//control arrows
	{
		word = _getch();
		GetConsoleScreenBufferInfo(console, currentPos);
		pos.X = currentPos->dwCursorPosition.X;
		pos.Y = currentPos->dwCursorPosition.Y;

		switch (word)
		{
		case 75:
			pos.X == 0 ? pos.X : pos.X--;
			SetConsoleCursorPosition(console, pos);
			break;
		case 72:
			pos.Y == 0 ? pos.Y : pos.Y--;
			SetConsoleCursorPosition(console, pos);
			break;
		case 77:
			pos.X == 79 ? pos.X : pos.X++;
			SetConsoleCursorPosition(console, pos);
			break;
		case 80:
			pos.Y++;
			SetConsoleCursorPosition(console, pos);
		}

	}
	else if (word == 13)	//enter
	{
		cout << endl;
	}
	else if (word == 27)	//esc
	{
		return 0;
	}
	else cout << (char)word;
}

size_t FSEmulator::getFreeBlockOffset()
{
	for (int i = 0; i < BLOCKSCOUNT; i++)
		if (fileTable.freeBlocksOffsets[i] != 0)
		{
			size_t temp = fileTable.freeBlocksOffsets[i];
			fileTable.freeBlocksOffsets[i] = 0;
			fileTable.freeSpace -= BLOCKSIZE;
			return temp;
		}
	return 0;
}

Entity FSEmulator::getFolder(string folderName)
{
	size_t offset = getFolderOffset(folderName);
	if (offset != 0)
		return readEntity(offset);
	return Entity();
}

size_t FSEmulator::getFolderOffset(string folderName)
{
	Entity folder;
	vector<string> foldersParts = splitFolderName("/" + folderName);
	for (int i = 0; i < MAXFILECOUNT; i++) {
		if (fileTable.filesOffsets[i] != 0)
		{
			folder = readEntity(fileTable.filesOffsets[i]);
			if (strcmp(folder.name, foldersParts[0].c_str()) == 0 && strcmp(folder.type, ((string)FOLDER).c_str()) == 0)
			{
				if (foldersParts.size() == 1 || foldersParts[1] == "") {
					return fileTable.filesOffsets[i];
				}
				else return getSubFolderOffset(foldersParts, folder, 1);
			}
		}
	}
	return 0;
}

size_t FSEmulator::getSubFolderOffset(vector<string> folders, Entity baseFolder, int lvl)
{
	Entity folder;
	for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
	{
		folder = readEntity(baseFolder.blocks[i]);
		if (strcmp(folder.name, folders[lvl].c_str()) == 0 && strcmp(folder.type, ((string)FOLDER).c_str()) == 0) {
			if (folders.size() - 1 == lvl) {
				return baseFolder.blocks[i];
			}
			else return getSubFolderOffset(folders, folder, lvl + 1);
		}
	}
	return 0;
}

void FSEmulator::changeFolder(string newFolder)
{
	if (newFolder == "/")
	{
		pwd = "";
		return;
	}
	Entity folder = getFolder(pwd + "/" + newFolder);
	if (strcmp(folder.name, newFolder.c_str()) == 0)
		pwd += "/" + newFolder;
	else cout << " Folder doesn't exist" << endl;
}

vector<string> FSEmulator::splitFolderName(string folderName)
{
	int len = folderName.length();
	int i = 0,k = 0;
	vector<string> words(1);
	for (; i < len; ++i) {
		char a = folderName[i];
		if (folderName[i] != '/' && folderName[i] != '\0')
		{
			words[k].push_back(folderName[i]);
		}
		else if (words[k] != "")
		{
			string temp;
			words.push_back(temp);
			k++;
		}
	}
	return words;
}

string FSEmulator::getCurrentFolder()
{
	return pwd;
}

void FSEmulator::showFolder(Entity folder, int lvl)
{
	for (int i = 0; i < FOLDERFILESMAXNUMBER; i++)
	{
		if (folder.blocks[i] != 0)
		{
			Entity temp = readEntity(folder.blocks[i]);

			if (lvl == 0) cout << "   ";
			for (int j = 0; j < lvl; j++)  cout << "            ";
			cout << "  --->";

			if (strcmp(temp.type, ((string)FOLDER).c_str()) == 0) {
				cout << " <" << string(temp.type) << "> ";
				cout << string(temp.name) << endl;
				showFolder(temp, lvl + 1);
			}
			else {
				cout << " " << string(temp.name) << endl;
			}
		}
	}
}

void FSEmulator::format()
{
	FSDrive.close();
	fstream file(FSMAINFILE, ios::binary | ios::out | ios::trunc);
	file.write((char*)(new FileTable()), sizeof(FileTable));
	for (int i = 0; i < BLOCKSCOUNT; i++)
		file.write((char*)(new Block()), sizeof(Block));
	file.close();
	pwd = "";
	offset = 0;
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	FSDrive.open(FSMAINFILE, ios::binary | ios::in | ios::out);
	FSDrive.read((char*)&fileTable, sizeof(FileTable));
}

