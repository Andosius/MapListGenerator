#include "MainWorker.hpp"

#include <iostream>

int main()
{

	MainWorker worker = MainWorker();

	if (worker.LoadConfigurations())
	{
		worker.PrepareParsing();

		worker.StartProcedure();


		int elements = worker.GetTotalObjectCount();

		int last_remaining = elements;
		int remaining = elements;

		while (worker.GetRemainingObjects() != 0)
		{
			last_remaining = remaining;
			remaining = worker.GetRemainingObjects();

			if (last_remaining != remaining)
				std::cout << "[PROGRESS] " << remaining << " of " << elements << " archives left (" << worker.GetProcessedCount() << "/" << elements << ")!" << std::endl;
		}

		worker.WriteResult("map_info.json");
	}


	return 0;
}