#pragma once

#include <pugixml.hpp>

#include <string>
#include <array>
#include <io.h>
#include <regex>


std::regex SevenZipFileOutputFilter = std::regex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} \S+ +\d+ +\d+\s+(.+))");

namespace Utility
{
	std::string GetCommandResult(const std::string command)
	{
		std::string result;
		std::array<char, 256> buffer = {};

		// Open command pipe
		FILE* pipe = _popen(command.c_str(), "r");

		if (pipe)
		{
			while (fgets(buffer.data(), (int)buffer.size(), pipe) != nullptr)
			{
				result += buffer.data();
			}
			_pclose(pipe);
		}

		return result;
	}

	bool IsValidMetaFile(const std::string& content)
	{
		if (content.find("<info") != -1)
		{
			return true;
		}
		return false;
	}

	std::string GetFilteredOutputLine(const std::string& line)
	{
		std::string result;
		std::smatch matches;

		if (std::regex_search(line, matches, SevenZipFileOutputFilter) && matches.size() > 1) {
			result = matches[1].str();
		}
		return result;
	}

	std::string ExtractMapName(const std::string& xmlContent)
	{
		pugi::xml_document doc;
		if (!doc.load_string(xmlContent.c_str()))
			return "";

		pugi::xml_node infoNode = doc.select_node("/meta/info").node();
		if (!infoNode)
			return "";

		pugi::xml_attribute nameAttribute = infoNode.attribute("name");
		if (!nameAttribute)
			return "";

		std::string mapName = nameAttribute.value();
		return mapName;
	}
}