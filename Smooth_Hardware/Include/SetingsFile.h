#pragma once

#include <File.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <map>

namespace SmoothHardware
{
	typedef map<std::wstring, std::wstring> Settings;

	class SettingsFile : public File
	{
		Settings settings;

	public:
		SettingsFile(Path path) : 
			File(path)
		{
			unsigned int keyEnd = 0;
			unsigned int start = 0;
			for (unsigned int i = 0; i < fileData.size(); i++)
			{
				if (fileData[i] == L':')
				{
					keyEnd = i;
				}
				else if (fileData[i] == L';')
				{
					auto key = fileData.substr(start, keyEnd - start);
					settings[key] = fileData.substr(keyEnd + 1, i - keyEnd - 1);
					start = i + 1;
				}
			}
		}

		void SetSetting(std::wstring key, std::wstring value)
		{
			this->settings[key] = value;
			this->SaveData();
		}

		void SaveData()
		{
			this->fileData = L"";

			for (auto itr = this->settings.begin(); itr != this->settings.end(); ++itr)
			{
				this->fileData += itr->first + L":" + itr->second + L";";
			}
		}

		Settings* getSettings()
		{
			return &settings;
		}
	};
}