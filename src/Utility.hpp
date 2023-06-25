#pragma once

#include <pugixml.hpp>

#include "concurrentqueue.h"

#include <string>
#include <array>
#include <io.h>
#include <regex>


namespace Utility
{
	/*void SetSevenZipPath(std::string& sevenZipPath);

	std::string GetCommandResult(const std::string command);

	bool IsValidMetaFile(const std::string& content);

	std::string GetFilteredOutputLine(const std::string& line);

	std::string ExtractMapName(const std::string& xmlContent);

	int GetAllArchives(const std::string& base_path, moodycamel::ConcurrentQueue<std::string>& queue);

	bool IsZipArchive(const std::string& filepath);

	std::vector<std::string> GetMetaFiles(const std::string& archive);

	std::string GetMetaFileContent(const std::string& archive, const std::string& file);

	int CalculateThreads();*/

	// REWRITE
	int CalculateThreads();
	bool IsAnyArchive(const std::string& filepath);
	bool IsZipArchive(const std::string& filepath);

	std::string ExtractMapName(const std::string& xmlContent);

	int GetAllArchives(const std::string& base_path, moodycamel::ConcurrentQueue<std::string>& queue);
	void GetArchiveMetaFileContents(const std::string& archive_path, std::vector<std::string>& file_contents);
}