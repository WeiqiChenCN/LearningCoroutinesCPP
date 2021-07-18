// win32.synchapi.waitabletimer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <stdio.h>
#include <iostream>

#include <Windows.h>

int main()
{
	HANDLE hTimer = NULL;
	LARGE_INTEGER liDueTime;
	/* in 100 nanosecond */
	//1 seconds = 1000 ms = 1000_000 us = 1000_000_000 ns 
	liDueTime.QuadPart = -5 * 10000000;

	// Create an unnamed waitable timer.
	hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (NULL == hTimer)
	{
		printf("CreateWaitableTimer failed (%d)\n", GetLastError());
		return 1;
	}

	printf("Waiting for 10 seconds...\n");

	// Set a timer to wait for 10 seconds.
	if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
	{
		printf("SetWaitableTimer failed (%d)\n", GetLastError());
		return 2;
	}

	// Wait for the timer.
	while (true)
	{
		auto waited = WaitForSingleObject(hTimer, 0);
		if (waited != WAIT_OBJECT_0) {
			printf("WaitForSingleObject failed (%x) (%d)\n", waited, GetLastError());
			Sleep(1000);
		}
		else
		{
			printf("Timer was signaled (%x).\n", waited);
			break;
		}
	}

	CloseHandle(hTimer);
	return 0;
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
