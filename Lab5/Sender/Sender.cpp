#include <iostream>
#include <Windows.h>
#include <string>

#include "../Receiver/Global_names.h"

HANDLE open_event(int id)
{
	HANDLE hEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, (EVENT_NAME + std::to_string(id)).c_str());

	if (hEvent == NULL)
	{
		std::cout << "Error with opening event: " << GetLastError() << std::endl;
	}
	return hEvent;
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

void open_semaphore(HANDLE& sem, std::string name)
{
	sem = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, (LPCSTR)(name.c_str()));

	if (sem == NULL)
	{
		std::cout << "Error with creating semaphore " << name << ":" << GetLastError() << std::endl;
	}
}

void open_semaphores(HANDLE& read_sem, HANDLE& write_sem)
{
	open_semaphore(read_sem, READ_SEMAPHORE);
	open_semaphore(write_sem, WRITE_SEMAPHORE);
}

HANDLE open_mutex()
{
	HANDLE mut = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());

	if (mut == NULL)
	{
		std::cout << "Error with opening mutex: " << GetLastError();
	}
	return mut;
}

void start_action(HANDLE write_semaphores, HANDLE read_semaphores, char* file_name)
{
	FILE* out = NULL;
	char message[20];

	HANDLE mutex = open_mutex();

	while (true)
	{
		std::cout << "Choose action: 1-write, 2-stop" << std::endl;
		char answer;
		std::cin >> answer;

		int page = 0;
		if (answer == '1')
		{
			fgets(message, 2, stdin);

			std::cout << "Enter message: " << std::endl;
			fgets(message, 20, stdin);

			WaitForSingleObject(write_semaphores, INFINITE);
			WaitForSingleObject(mutex, INFINITE);

			out = open_file(file_name);

			rewind(out);
			fread(&page, sizeof(int), 1, out);

			fseek(out, sizeof(int) + sizeof(char) * 20 * page, SEEK_SET);
			printf("\nMessage: %s\n\n", message);

			for (int i = 0; i < strlen(message); i++)
				fwrite(&message[i], sizeof(char), 1, out);

			page = (page + 1) % 20;

			rewind(out);
			fwrite(&page, sizeof(int), 1, out);

			fclose(out);

			ReleaseMutex(mutex);
			ReleaseSemaphore(read_semaphores, 1, NULL);
		}
		else if (answer == '2')
		{
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	SetEvent(open_event(atoi(argv[2])));

	HANDLE read_semaphores = NULL, write_semaphores = NULL;
	open_semaphores(read_semaphores, write_semaphores);

	start_action(write_semaphores, read_semaphores, argv[1]);
}