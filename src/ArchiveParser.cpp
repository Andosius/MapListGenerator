#include "ArchiveParser.hpp"

#include "Utility.hpp"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <fstream>


ArchiveParser::ArchiveParser(const std::string& base_directory, const int threads)
	: m_BasePath(base_directory), m_ThreadPool(threads)
{
	GetAllArchives();
}

ArchiveParser::~ArchiveParser()
{
	m_ThreadPool.Shutdown();
}

void ArchiveParser::CheckResults()
{
	auto it = m_Futures.begin();

	while (it != m_Futures.end())
	{
		if (it->valid())
		{
			if (it->wait_for(std::chrono::microseconds(1)) == std::future_status::ready)
			{
				nlohmann::json obj = it->get();

				if (obj["name"] == "")
					m_FailedArray.emplace_back(obj);
				else
					m_MapArray.emplace_back(obj);

				it = m_Futures.erase(it);
			}
			else
			{
				++it;
			}
		}
		else
		{
			++it;
		}
	}
}

void ArchiveParser::GetAllArchives()
{
	for (const auto& entry : std::filesystem::directory_iterator(m_BasePath))
	{
		if (entry.status().type() == std::filesystem::file_type::regular)
		{
			std::string filename(entry.path().string());

			if (IsValidArchiveType(filename))
				m_ArchivePaths.push_back(entry.path().string());
		}
	}
}

bool ArchiveParser::IsValidArchiveType(const std::string& filepath)
{
	std::size_t len = filepath.length();

	if (len >= 4)
	{
		std::string extension = filepath.substr(len - 4);

		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c)
			{
				return std::tolower(c);
			});

		return (extension == ".zip" || extension == ".rar");
	}
	return false;
}

size_t ArchiveParser::ProcessAll()
{
	for (const auto& archive : m_ArchivePaths)
		m_Futures.push_back(m_ThreadPool.AddTask(std::bind(&ArchiveParser::ProcessElement, this, archive)));

	return m_ArchivePaths.size();
}

nlohmann::json ArchiveParser::ProcessElement(const std::string& archive)
{
	std::string mapName;
	std::vector<std::string> meta_files = GetMetaFiles(archive);

	nlohmann::json mapData;
	mapData["path"] = archive;

	for (const auto& meta_file : meta_files)
	{
		std::string file = Utility::GetFilteredOutputLine(meta_file);
		std::string content = GetMetaFileContent(archive, file);

		if (Utility::IsValidMetaFile(content))
		{
			mapName = Utility::ExtractMapName(content);
			mapData["name"] = mapName;

			return mapData;
		}
	}
	mapData["name"] = "";
	return mapData;
}

std::vector<std::string> ArchiveParser::GetMetaFiles(const std::string& archive)
{
	std::vector<std::string> result;

	std::string command("7z.exe l " + archive + " -so");
	std::string contents = Utility::GetCommandResult(command);

	std::stringstream ss(contents);
	std::string line;

	while (std::getline(ss, line, '\n'))
	{
		if (line.find("meta.xml") != -1 && line.find("backup") == -1)
			result.push_back(line);
	}

	return result;
}

std::string ArchiveParser::GetMetaFileContent(const std::string& archive, const std::string& file)
{
	std::string command("7z.exe e -so -- \"" + archive + "\" \"" + file + "\"");
	std::string contents = Utility::GetCommandResult(command);

	return contents;
}

void ArchiveParser::WriteDataToFile(const std::string& outputFileName)
{
	nlohmann::json data;
	data["maps"] = m_MapArray;
	data["unresolved"] = m_FailedArray;

	std::string output = data.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore);

	std::ofstream outputStream(outputFileName, std::ios_base::app);

	outputStream << output << std::endl;
	outputStream.close();
}
