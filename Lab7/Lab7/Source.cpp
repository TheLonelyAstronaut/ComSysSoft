#include "FSEmulator.h"

string parseArguments(string& command) {
	string argument;

	auto pos = command.find(" ");

	if (pos != string::npos)
	{
		argument = command.substr(pos + 1);
		command = command.substr(0, pos);
	}

	return argument;
}

int main()
{
	string command, argument;
	FSEmulator FS;

	while (true)
	{
		cout << FS.getCurrentFolder() << ">> ";
		getline(std::cin >> std::ws, command);
		argument = parseArguments(command);

		if (command == "exit") break;
		
		if (command == "dir") 
		{ 
			FS.dir();
			continue;
		}

		if (command == "paste") 
		{ 
			FS.paste(); 
			continue; 
		}
		
		if (command == "format") 
		{ 
			FS.format(); 
			continue; 
		}
		
		if (command == "cls") 
		{
			system("cls"); 
			continue; 
		}

		if (argument == "") {
			cout << " Incorrect number of arguments or unknown command" << endl;
			continue;
		}

		if (command == "mkdir") 
		{ 
			FS.createEntity(FOLDER, argument);
			continue; 
		}
		
		if (command == "mkfile") 
		{ 
			FS.createEntity(FILE, argument); 
			continue; 
		}
		
		if (command == "cd") 
		{ 
			FS.changeFolder(argument); 
			continue; 
		}
		
		if (command == "rm") 
		{
			FS.remove(argument); 
			continue; 
		}
		
		if (command == "open") 
		{ 
			FS.openFile(argument);
			continue; 
		}
		
		if (command == "cut") 
		{ 
			FS.cut(argument);
			continue;
		}

		cout << " Unknown command" << endl;
	}
	return 0;
}
