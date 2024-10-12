#ifndef _STOP_WATCH_H
#define _STOP_WATCH_H

#include <stdio.h>
#include <time.h>
#include <string>

class StopWach
{
	clock_t ts;
	clock_t te;
	clock_t diff;
	std::string _name;
public:
	inline StopWach()
	{
		ts = clock();
		_name = "";
	}
	inline StopWach(const char* name)
	{
		ts = clock();
		_name = name;
	}

	inline void start()
	{
		ts = clock();
	}

	inline void stop()
	{
		te = clock();
		diff = te - ts;
		print();
	}

	inline long time_course()
	{
		return clock() - ts;
	}


	inline void print()
	{
		//printf("[%s] time %d %fsec %fmin\n", _name.c_str(), diff, (float)diff/(float)CLOCKS_PER_SEC, (float)diff/(60.0*(float)CLOCKS_PER_SEC));
	}

};

#endif
