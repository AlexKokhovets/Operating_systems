#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>

#include "Global_names.h"


void get_information(std::string& file_name, int& num_of_records, int& num_of_senders)
{
	std::cout << "Enter file_name: \n";
	std::getline(std::cin, file_name);

	std::cout << "Enter num of records: \n";
	std::cin >> num_of_records;

	std::cout << "Enter num of senders: \n";
	std::cin >> num_of_senders;
}

void create_file(std::string file_name, FILE** f)
{
	fopen_s(f, file_name.c_str(), "w+b");

	if (f == NULL)
	{
		std::cout << "Error with open file: " << GetLastError() << std::endl;
	}
}

FILE* open_file(char* file_name)
{
	FILE* f;
	fopen_s(&f, file_name, "r+b");

	if (f == NULL)
	{
		std::cout << "Error with opening file: " << GetLastError() << std::endl;
	}
	return f;
}

void make_arguments_for_process(std::string file_name, int id, std::string& answer)
{
	answer.append(" ");
	answer.append(file_name);
	answer.append(" ");
	answer.append(std::to_string(id));
}

void create_process(LPCSTR process_name, std::string fname, int id)
{
	std::string args = "";
	make_arguments_for_process(fname, id, args);

	STARTUPINFOA cif;
	ZeroMemory(&cif, sizeof(STARTUPINFOA));
	PROCESS_INFORMATION pi;

	if (!CreateProcessA((LPSTR)process_name, (LPSTR)args.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &cif, &pi))
	{
		std::cout << "Error with creating new process: " << GetLastError() << std::endl;
	}
}

void create_event_for_process(int id, HANDLE& hEvent)
{
	hEvent = CreateEventA(NULL, FALSE, FALSE, (EVENT_NAME+std::to_string(id)).c_str());

	if (hEvent == NULL)
	{
		std::cout << "Error with creating event for process: " << GetLastError();
	}
}

void create_semaphore(HANDLE& sem, std::string name, int start, int finish)
{
	sem = CreateSemaphoreA(NULL, start, finish, (LPCSTR)(name.c_str()));

	if (sem == NULL)
	{
		std::cout << "Error with creating semaphore " << name << ":" << GetLastError() << std::endl;
	}
}

void create_semaphores(HANDLE& read_sem, HANDLE& write_sem, int max_num)
{
	create_semaphore(read_sem, READ_SEMAPHORE, 0, max_num);
	create_semaphore(write_sem, WRITE_SEMAPHORE, max_num, max_num);
}

void start_action(FILE* read_file, std::string file_name, HANDLE read_semaphore, HANDLE write_semaphore)
{
	HANDLE cs_mutex = CreateMutexA(NULL, FALSE, MUTEX_NAME.c_str());

	char message[20];
	int page = 0;
	while (true)
	{
		std::cout << "Choose action: 1-read, 2-stop" << std::endl;
		char answer;
		std::cin >> answer;

		if (answer == '1')
		{
			WaitForSingleObject(read_semaphore, INFINITE);
			WaitForSingleObject(cs_mutex, INFINITE);
			read_file = open_file((char*)file_name.c_str());

			fseek(read_file, sizeof(int) + sizeof(char) * 20 * page, SEEK_SET);

			for (int i = 0; i < 20; i++)
			{
				fread(&message[i], sizeof(char), 1, read_file);
				if (message[i] == '\n')
				{
					message[i] = '\0';
					break;
				}
			}

			page = (page+1)%20;

			fclose(read_file);

			ReleaseMutex(cs_mutex);
			ReleaseSemaphore(write_semaphore, 1, NULL);
			printf("\nMessage: %s\n\n", message);
		}
		else if (answer == '2')
		{
			break;
		}
	}

}

int main()
{	
	std::string file_name;
	int num_of_records, num_of_senders;
	
	get_information(file_name, num_of_records, num_of_senders);

	FILE* read_file = NULL;
	create_file(file_name, &read_file);

	fclose(read_file);                                                                 

	HANDLE read_semaphore = NULL, write_semaphore = NULL;
	create_semaphores(read_semaphore, write_semaphore, num_of_records);

	std::vector<HANDLE> process_events(num_of_senders);
	for (int i = 0; i < num_of_senders; ++i)
	{
		create_event_for_process(i, process_events[i]);
		create_process("../x64/Debug/Sender.exe", file_name, i);
	}
	WaitForMultipleObjects(num_of_senders, process_events.data(), TRUE, INFINITE);

	start_action(read_file, file_name, read_semaphore, write_semaphore);

	system("pause");
}