
#include <windows.h>
#include <ctime>

#include "predef.h"
#include "stress.h"

int main()
{
	srand(time(NULL));


	Stress stress;
	stress.init();

	while (1)
	{
		stress.run();
		Sleep(1);
	}


	return 0;
}