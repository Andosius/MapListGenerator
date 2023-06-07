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
	size_t GetOutstandingElementSize() { return m_Futures.size(); }
	void CheckResults();

	size_t ProcessAll();
	void WriteDataToFile(const std::string& outputFileName);
	

private:
	void GetAllArchives();
	bool IsValidArchiveType(const std::string& filepath);
	nlohmann::json ProcessElement(const std::string& archive);


// Processing functions
private:
	std::vector<std::string> GetMetaFiles(const std::string& archive);
	std::string GetMetaFileContent(const std::string& archive, const std::string& file);

private:
	std::string m_BasePath;
	std::vector<std::string> m_ArchivePaths;

	ThreadPool m_ThreadPool;

	std::vector<std::future<nlohmann::json>> m_Futures;

	nlohmann::json m_MapArray = nlohmann::json::array();
	nlohmann::json m_FailedArray = nlohmann::json::array();
};

