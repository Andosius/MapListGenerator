#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ThreadPool.hpp"


class ArchiveParser
{
public:
	ArchiveParser(const std::string& base_directory, const int threads);
	~ArchiveParser();

	size_t GetQueueSize() { return m_ThreadPool.QueueSize(); }

	size_t ProcessAll();
	void WriteDataToFile(const std::string& outputFileName);

	bool IsDone();
	

private:
	void GetAllArchives();
	bool IsValidArchiveType(const std::string& filepath);
	void ProcessElement(const std::string& archive);


// Processing functions
private:
	std::vector<std::string> GetMetaFiles(const std::string& archive);
	std::string GetMetaFileContent(const std::string& archive, const std::string& file);

private:
	std::string m_BasePath;
	std::vector<std::string> m_ArchivePaths;

	ThreadPool m_ThreadPool;

	std::mutex m_Mutex;

	nlohmann::json m_MapArray = nlohmann::json::array();
	nlohmann::json m_FailedArray = nlohmann::json::array();
};

