#pragma once

#include <Path.h>

#include <iostream>

namespace SmoothHardware
{
	class File
	{
	protected:
		Path path;
		std::wstring fileData;

	public:
		File(Path path)
		{
			this->path = path;

			std::ifstream t(path);

			if (t.is_open())
			{
				t.seekg(0, std::ios::end);
				fileData.reserve(t.tellg());
				t.seekg(0, std::ios::beg);

				fileData.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());

				t.close();
			}
		}

		void Save()
		{
			std::ofstream outfile(path, std::ofstream::trunc | std::ofstream::out);

			std::string str(this->fileData.begin(), this->fileData.end());
			outfile.write(str.c_str(), str.size());

			outfile.close();
		}
	};
}