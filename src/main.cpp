#include "ArchiveParser.hpp"

#include <iostream>

int CalculateThreads();


int main()
{
	int threads = CalculateThreads();
	std::cout << "[MAP-LIST-GENERATOR] Processing archives with " << threads << " threads." << std::endl;

	ArchiveParser parser = ArchiveParser(R"(\\nas\Denis\Maps\)", threads);
	size_t elements = parser.ProcessAll();

	size_t last_remaining = elements;
	size_t remaining = elements;

	while (parser.GetQueueSize() > 0)
	{
		last_remaining = remaining;
		remaining = parser.GetQueueSize();

		if (last_remaining != remaining)
			std::cout << "[PROGRESS] " << remaining << " of " << elements << " archives left (" << remaining << "/" << elements << ")!" << std::endl;
	}
	
	while (!parser.IsDone())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	parser.WriteDataToFile("map_info.json");
}

int CalculateThreads()
{
	int threads = std::thread::hardware_concurrency();

	if (!threads)
		threads = 1;
	else
		threads = threads / 2;

	return threads;
}