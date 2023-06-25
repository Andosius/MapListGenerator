#include "MainWorker.hpp"

#include "Utility.hpp"

#include <fstream>
#include <iostream>

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
	if (std::filesystem::is_regular_file(CONFIG_FILE))
	{
		std::ifstream configFile;
		configFile.open(CONFIG_FILE);

		std::string content = "";
		std::string line;

		while (std::getline(configFile, line) && configFile.good())
		{
			content += line;
		}
		configFile.close();


		nlohmann::json config;
		try {
			config = nlohmann::json::parse(content);
		}
		catch (nlohmann::json::parse_error& e)
		{
			std::cout << "[ERROR] Unable to read config.json file, invalid data. Aborting!" << std::endl;
			std::cout << e.what() << std::endl;

			exit(EXIT_FAILURE);
		}

		m_ParsePath = config["path"].get<std::string>();
		return true;
	}
	else
	{
		nlohmann::json config;
		config["path"] = "";

		std::ofstream configFile;
		configFile.open(CONFIG_FILE, std::ios_base::app);

		if (configFile.is_open())
		{
			std::string dump = config.dump(4);
			configFile << dump << std::endl;

			configFile.close();

			std::cout << "[INFO] Please configure the program, take a look at " << CONFIG_FILE << "!" << std::endl;
			exit(EXIT_SUCCESS);

		}

		return true;
	}
	return false;
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

#if 0
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
