#include <Windows.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <fstream>

struct Request
{
	std::vector<int> const massive;
	double* answer;

	Request(std::vector<int> const& mas): massive(mas)
	{}
};

DWORD WINAPI min_max(LPVOID param)
{
	Request* req = (Request*)param;

	int min = req->massive[0];
	int max = min;
	for (auto a: req->massive)
	{
		if (a > max)
			max = a;

		Sleep(7);
		if (a < min)
			min = a;

		Sleep(7);
	}

	req->answer[0] = min;
	req->answer[1] = max;
	return 0;
}
DWORD WINAPI average(LPVOID param)
{
	Request* req = (Request*)param;

	if (req->massive.size() == 0)
		return 1;

	int average = 0;
	for (auto a: req->massive)
	{
		average += a;
		Sleep(12);
	}

	req->answer[0] = (double)average/req->massive.size();
	return 0;
}

int wait_thread_and_close_handle(HANDLE t1, HANDLE t2)
{
	WaitForSingleObject(t1, INFINITE);
	WaitForSingleObject(t2, INFINITE);
	CloseHandle(t1);
	CloseHandle(t2);
	return 0;
}
int change_minmax_to_average(std::vector<int>& mas, double min, double max, double average)
{
	for (auto& a : mas)
	{
		if (a == min || a == max)
		{
			a = (int)average;
		}
	}
	return 0;
}
int print_vector(std::vector<int>& mas)
{
	std::cout << "New massive: ";
	std::copy(mas.begin(), mas.end(), std::ostream_iterator<int>(std::cout, " "));
	std::cout << std::endl;
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

	Request r1 = Request(mas), r2 = Request(mas);
	r1.answer = new double[1];
	r2.answer = new double[2];

	HANDLE min_max_thread = CreateThread(NULL, 0, min_max, (LPVOID)&r1, 0, NULL);
	HANDLE average_thread = CreateThread(NULL, 0, average, (LPVOID)&r2, 0, NULL);

	if (min_max_thread == NULL || average_thread == NULL) 
	{
		std::cout << GetLastError();
		return 1;
	}
	wait_thread_and_close_handle(min_max_thread, average_thread);

	std::cout << "Average: " << r2.answer[0] << std::endl;
	std::cout << "Min: " << r1.answer[0] << " Max:" << r1.answer[1] << std::endl;

	change_minmax_to_average(mas, r1.answer[0], r1.answer[1], r2.answer[0]);
	print_vector(mas);
	return 0;
}