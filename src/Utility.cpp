#pragma once

#include "Utility.hpp"

#include <minizip-ng/mz.h>
#include <minizip-ng/mz_strm.h>
#include <minizip-ng/mz_zip.h>
#include <minizip-ng/mz_zip_rw.h>

#include <filesystem>
#include <sstream>
#include <thread>
#include <iostream>


namespace Utility
{
	int CalculateThreads()
	{
		int threads = std::thread::hardware_concurrency();

		if (!threads)
			threads = 1;
		else
			threads = threads / 2;

		return threads;
	}

	bool IsAnyArchive(const std::string& filepath)
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

	bool IsZipArchive(const std::string& filepath)
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

			return (extension == ".zip");
		}
		return false;
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

	int GetAllArchives(const std::string& base_path, moodycamel::ConcurrentQueue<std::string>& queue)
	{
		int count = 0;

		for (const auto& entry : std::filesystem::directory_iterator(base_path))
		{
			if (entry.status().type() == std::filesystem::file_type::regular)
			{
				std::string filename(entry.path().string());

				if (IsAnyArchive(filename))
				{
					queue.enqueue(entry.path().string());
					count++;
				}
			}
		}
		return count;
	}

	void GetArchiveMetaFileContents(const std::string& archive_path, std::vector<std::string>& file_contents)
	{

		void* zip_reader = mz_zip_reader_create();

		if (mz_zip_reader_open_file_in_memory(zip_reader, archive_path.c_str()) == MZ_OK)
		{
			if (mz_zip_reader_goto_first_entry(zip_reader) == MZ_OK)
			{
				do
				{

					if (mz_zip_reader_entry_is_dir(zip_reader) != MZ_OK)
					{

						mz_zip_file* file_info = NULL;
						if (mz_zip_reader_entry_get_info(zip_reader, &file_info) == MZ_OK)
						{
							std::string filename(file_info->filename);

							if (filename.find("meta.xml") != std::string::npos)
							{
								mz_zip_reader_set_encoding(zip_reader, MZ_ENCODING_UTF8);

								int32_t buffer_size = mz_zip_reader_entry_save_buffer_length(zip_reader);

								char* buffer = (char*)malloc(buffer_size);

								if (buffer != NULL)
								{
									memset(buffer, 0, buffer_size);
									if (mz_zip_reader_entry_save_buffer(zip_reader, buffer, buffer_size) == MZ_OK)
									{
										std::string file_content = std::string(buffer);
										file_content = file_content.substr(0, buffer_size);
										file_contents.push_back(file_content);

									} else std::cout << "Can't save entry to buffer in file: \"" << archive_path << "\"!" << std::endl;

									free(buffer);
								}


							}

						} else std::cout << "Can't get file_info in file: \"" << archive_path << "\"!" << std::endl;

					}


				} while (mz_zip_reader_goto_next_entry(zip_reader) == MZ_OK);
			} else std::cout << "Can't go to first entry in file: \"" << archive_path << "\"!" << std::endl;

			mz_zip_reader_close(zip_reader);
		} else std::cout << "Can't open file: \"" << archive_path << "\" in memory!" << std::endl;

		mz_zip_reader_delete(&zip_reader);

	}

}