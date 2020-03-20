#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <Windows.h>

CRITICAL_SECTION* sections;

struct request
{
	int num;
	int is_finished;
	std::vector<int>* mas;

	HANDLE continue_event;
	HANDLE start_thread_event;
	HANDLE finish_event;
	HANDLE ready_event;
};

int change_and_sleep(request* req, int tmp)
{
	Sleep(5);
	req->mas->operator[](tmp) = req->num;
	LeaveCriticalSection(&sections[tmp]);
	Sleep(5);
	return 0;
}
int set_event_and_wait(HANDLE ready_event, HANDLE wait_event)
{
	SetEvent(ready_event);
	WaitForSingleObject(wait_event, INFINITE);
	ResetEvent(wait_event);
	return 0;
}
request make_request(int num, HANDLE start_event, std::vector<int>& mas)
{
	request req;

	req.mas = &mas;
	req.num = num + 1;
	req.start_thread_event = start_event;
	req.is_finished = 0;

	req.continue_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	req.ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	req.finish_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	return req;
}

int finish_thread(request* r)
{
	if (r->is_finished == 1)
	{
		return 1;
	}

	r->is_finished = 1;

	set_event_and_wait(r->continue_event, r->finish_event);
	return 0;
}
int continue_threads(std::vector<request>& mas)
{
	for (auto a : mas)
	{
		if (a.is_finished == 0)
			SetEvent(a.continue_event);
	}
	return 0;
}

DWORD WINAPI market(LPVOID param)
{
	request* req = (request*)param;

	WaitForSingleObject(req->start_thread_event, INFINITE);

	srand(req->num);

	int count = 0;
	while (true)
	{
		int tmp = rand() % req->mas->size();
		EnterCriticalSection(&sections[tmp]);
		if (req->mas->at(tmp) == 0)
		{
			change_and_sleep(req, tmp);
			count++;
		}
		else
		{
			LeaveCriticalSection(&sections[tmp]);
			std::cout << req->num << " " << count << " " << tmp << std::endl;
			
			set_event_and_wait(req->ready_event, req->continue_event);

			if (req->is_finished)
				break;

			ResetEvent(req->ready_event);
		}
	}

	for (auto& a: *req->mas)
	{
		if (a == req->num)
			a = 0;
	}

	SetEvent(req->finish_event);
	return 0;
}

int main()
{
	int size;
    std::cout << "Enter size of massive\n";
	std::cin >> size;
	if (size <= 0)
	{
		std::cout << "Invalid size\n";
		return 1;
	}

	std::vector<int> mas(size, 0);

	int num_of_threads;
	std::cout << "Enter num of threads\n";
	std::cin >> num_of_threads;
	if (size <= 0)
	{
		std::cout << "Invalid num of threads\n";
		return 1;
	}

	std::vector<HANDLE> threads (num_of_threads);
	std::vector<request> requests (num_of_threads);

	HANDLE start_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	sections = new CRITICAL_SECTION[size];

	for (int i = 0; i < num_of_threads; i++)
	{
		InitializeCriticalSection(&sections[i]);
		requests[i] = make_request(i, start_event, mas);
		threads[i] = CreateThread(NULL, 0, market, (LPVOID)&requests[i], 0, NULL);
	}

	SetEvent(start_event);

	for (int i = 0; i < num_of_threads; i++)
	{
		for (auto a: requests)
		{
			if(a.is_finished == 0)
				WaitForSingleObject(a.ready_event, INFINITE);
			ResetEvent(a.ready_event);
		}

		int n;
		std::cout << "Enter thread:\n";
		std::cin >> n;
		n--;

		if (n >= num_of_threads || finish_thread(&requests[n]) == 1)
		{
			i++;
			continue;
		}

		std::cout << "Massive: " ;
		std::copy(mas.begin(), mas.end(), std::ostream_iterator<int>(std::cout, " "));
		std::cout << std::endl;

		continue_threads(requests);
	}
	delete[] sections;
}