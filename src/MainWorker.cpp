#include "MainWorker.hpp"

#include "Utility.hpp"

#include <fstream>
#include <iostream>

#define DEBUG_OUTPUT


MainWorker::MainWorker()
	: m_ThreadCount(Utility::CalculateThreads())
{

}

MainWorker::~MainWorker()
{

}

void MainWorker::PrepareParsing()
{
	m_ArchiveCount = Utility::GetAllArchives(m_ParsePath, m_Producer);
}

void MainWorker::AddTask(const std::string& archive)
{
	m_Producer.enqueue(archive);
}

void MainWorker::StartProcedure()
{
	// 3/4 of available threads for working and the threads left for writing
	int worker_count = (m_ThreadCount * 3) / 4;
	int writer_count = m_ThreadCount - worker_count;

	for (int i = 0; i != worker_count; i++)
	{
		m_Threads.push_back(std::thread(&MainWorker::ParseArchives, this));
	}

	for (int i = 0; i != writer_count; i++)
	{
		m_Threads.push_back(std::thread(&MainWorker::PushResults, this));
	}
}

void MainWorker::DestroyThreads()
{
	for (auto& thread : m_Threads)
	{
		thread.join();
	}
}

bool MainWorker::LoadConfigurations()
{
	nlohmann::json config;

	if (!std::filesystem::exists(CONFIG_FILE))
	{
		config["path"] = "";

		std::ofstream file(CONFIG_FILE);
		file << config.dump(4) << std::endl;

		std::cout << "[INFO] Please configure the program, take a look at " << CONFIG_FILE << "!" << std::endl;

		return false;
	}


	std::ifstream file(CONFIG_FILE);

	file >> config;
	m_ParsePath = config["path"].get<std::string>();

	return true;
}

void MainWorker::WriteResult(const char* filename)
{
	if (m_ArchiveCount == m_ArchiveProcessedCounter)
	{
		nlohmann::json data;
		data["maps"] = m_MapArray;
		data["unresolved"] = m_FailedArray;

		std::string output = data.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore);

		std::ofstream outputStream(filename, std::ios_base::app);

		outputStream << output << std::endl;
		outputStream.close();
	}
}

void MainWorker::ParseArchives()
{

	std::string archive;
	while (m_Producer.try_dequeue(archive))
	{
		nlohmann::json map;
		map["name"] = "";
		map["path"] = archive;

		if (Utility::IsZipArchive(archive))
		{
			std::vector<std::string> file_contents;
			Utility::GetArchiveMetaFileContents(archive, file_contents);

			for (const auto& content : file_contents)
			{
				std::string mapName = Utility::ExtractMapName(content);

				if (mapName != "")
					map["name"] = mapName;
				
			}

#ifdef DEBUG_OUTPUT
			if (map["name"] == "")
			{
				std::filesystem::path p(archive);

				std::string logname = "logs/" + p.filename().string() + ".txt";

				std::ofstream log(logname);
				
				for (const auto& content : file_contents)
				{
					log << content << std::endl;
					log << "----------------------------------------" << std::endl;
				}

				log.close();

			}
#endif
		}

		m_Consumer.enqueue(map);
	}
}

void MainWorker::PushResults()
{
	while (m_ArchiveProcessedCounter != m_ArchiveCount)
	{
		nlohmann::json data;
		if (m_Consumer.try_dequeue(data))
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			if (data["name"].get<std::string>() != "")
			{
				m_MapArray.push_back(data);
			}
			else
			{
				m_FailedArray.push_back(data);
			}

			m_ArchiveProcessedCounter++;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
