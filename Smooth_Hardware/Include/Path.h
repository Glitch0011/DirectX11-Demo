#pragma once

#include <string>

using namespace std;

namespace SmoothHardware
{
	class Path
	{
		wstring path;
	public:
		
		Path()
		{
			path = L"";
		}

		Path(wstring _path)
		{
			this->path = _path;
		}

		bool has_extension()
		{
			return this->extension().length() > 0;
		}

		wstring file_without_extension()
		{
			auto idx = this->path.rfind('.');

			if (idx != string::npos)
			{
				return path.substr(0, idx);
			}
			else
			{
				return path;
			}
		}

		wstring file_name()
		{
			auto idx = this->file_without_extension().rfind('\\');

			if (idx != string::npos)
			{
				return this->file_without_extension().substr(idx + 1); 
			}
			else
			{
				return this->file_without_extension();
			}
		}

		wstring extension()
		{
			auto idx = this->path.rfind('.');

			if (idx != string::npos)
			{
				return path.substr(idx + 1);
			}
			else
			{
				return L"";
			}
		}

		operator wstring() const
		{
			return this->path;
		}

		wstring Get()
		{
			return this->path;
		}
	};
}