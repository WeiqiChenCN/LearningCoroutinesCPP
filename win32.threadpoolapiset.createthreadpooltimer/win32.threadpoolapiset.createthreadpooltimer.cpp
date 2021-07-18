// win32.threadpoolapiset.createthreadpooltimer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <thread>
#include <iostream>

#include <Windows.h>

struct Data {
	std::string name;
	static VOID CALLBACK TimerCallback(
		_Inout_     PTP_CALLBACK_INSTANCE Instance,
		_Inout_opt_ PVOID                 Context,
		_Inout_     PTP_TIMER             Timer
	)
	{
		auto me = (Data*)Context;
		std::cout << "Timer expires, " << me->name << " in thread " 
			<< std::this_thread::get_id() << std::endl;
		CloseThreadpoolTimer(Timer);
	}
};

int main()
{
	auto data = new Data{"Timer's Name"};
	auto timer = CreateThreadpoolTimer(
		data->TimerCallback,
		data, nullptr);
	if (timer == nullptr)
	{
		throw new std::runtime_error("timer created failure.");
	}
	DWORD milliseconds = 2000;
	auto ft64 = -1 * static_cast<INT64>(milliseconds) * 10000;
	FILETIME ft;
	memcpy(&ft, &ft64, sizeof(INT64));
	SetThreadpoolTimer(timer, &ft, 0, 0);


	std::cout << "Hello World!\n";
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::cout << "Thread " << std::this_thread::get_id() << " exit"
			  << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
