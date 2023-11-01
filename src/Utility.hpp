#pragma once

#include <pugixml.hpp>

#include "concurrentqueue.h"

#include <string>
#include <array>
#include <io.h>
#include <regex>


namespace Utility
{
	int CalculateThreads();
	bool IsAnyArchive(const std::string& filepath);
	bool IsZipArchive(const std::string& filepath);

	std::string ExtractMapName(const std::string& xmlContent);

	int GetAllArchives(const std::string& base_path, moodycamel::ConcurrentQueue<std::string>& queue);
	void GetArchiveMetaFileContents(const std::string& archive_path, std::vector<std::string>& file_contents);
}
