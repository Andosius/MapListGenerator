#pragma once

#include "concurrentqueue.h"
#include "nlohmann/json.hpp"

#include <atomic>
#include <mutex>


#define CONFIG_FILE "config.json"


class MainWorker
{
public:
	MainWorker();
	~MainWorker();

	void PrepareParsing();

	void AddTask(const std::string& archive);
	void StartProcedure();
	void DestroyThreads();

	bool LoadConfigurations();

	void WriteResult(const char* filename);
	void WriteResult(std::string& filename) { WriteResult(filename.c_str()); }

	inline int GetRemainingObjects() const { return m_ArchiveCount - m_ArchiveProcessedCounter; }
	inline int GetTotalObjectCount() const { return m_ArchiveCount; }
	inline int GetProcessedCount() const { return m_ArchiveProcessedCounter; }

// Multi Threading
private:
	void ParseArchives();
	void PushResults();

public:
	std::atomic<int> ElementsLeft = 0;

private:
	moodycamel::ConcurrentQueue<std::string> m_Producer;
	moodycamel::ConcurrentQueue<nlohmann::json> m_Consumer;

	std::string m_ParsePath;
	std::string m_SevenZipPath;
	int m_ThreadCount;

	std::vector<std::string> m_ArchivePaths;
	int m_ArchiveCount = 0;
	std::atomic<int> m_ArchiveProcessedCounter;

	std::mutex m_Mutex;
	std::vector<std::thread> m_Threads;

	nlohmann::json m_MapArray = nlohmann::json::array();
	nlohmann::json m_FailedArray = nlohmann::json::array();
};