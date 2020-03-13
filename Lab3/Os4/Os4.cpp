#include <windows.h>
#include <iostream>
#include <vector>

struct Request
{
	std::vector<int>* massive;
	double answer1;
	double answer2;
};

DWORD WINAPI min_max(LPVOID param)
{
	Request* req = (Request*)param;

	int min = req->massive->at(0);
	int max = min;
	for (size_t i = 0; i < req->massive->size(); i++)
	{
		int tmp = req->massive->at(i);
		if (tmp > max)
		{
			max = tmp;
		}
		Sleep(7);
		if (tmp < min)
		{
			min = tmp;
		}
		Sleep(7);
	}

	req->answer1 = min;
	req->answer2 = max;

	std::cout << "Min: " << min << " Max: " << max << std::endl;

	return 0;
}
DWORD WINAPI average(LPVOID param)
{
	Request* req = (Request*)param;

	int average = 0;
	for (size_t i = 0; i < req->massive->size(); i++)
	{
		average += req->massive->at(i);
		Sleep(12);
	}

	req->answer1 = (double)average/req->massive->size();

	std::cout << "Average: " << req->answer1 << std::endl;

	return 0;
}

int main()
{
	int size;
	std::cout << "Enter size of massive:\n";
	std::cin >> size;
	std::vector<int> mas(size);

	std::cout << "Enter elements of massive: ";

	for (int i = 0; i < size; i++)
	{
		std::cin >> mas[i];
	}

	Request r1, r2;
	r1.massive = &mas;
	r2.massive = &mas;

	HANDLE min_max_thread, average_thread;
	DWORD IDThread[2];
	min_max_thread = CreateThread(NULL, 0, min_max, (LPVOID)&r1, 0, &IDThread[0]);
	average_thread = CreateThread(NULL, 0, average, (LPVOID)&r2, 0, &IDThread[1]);

	if (min_max_thread == NULL || average_thread == NULL) 
	{
		std::cout << GetLastError();
		return 1;
	}
	WaitForSingleObject(min_max_thread, INFINITE);
	WaitForSingleObject(average_thread, INFINITE);
	CloseHandle(min_max_thread);
	CloseHandle(average_thread);

	for (int i = 0; i < size; i++)
	{
		if (mas.at(i) == r1.answer1 || mas.at(i) == r1.answer2)
		{
			mas.at(i) = (int)r2.answer1;
		}
	}

	std::cout << "New massive: " << std::endl;
	for (int i = 0; i < size; i++)
	{
		std::cout << mas.at(i) << " ";
	}

	return 0;
}
