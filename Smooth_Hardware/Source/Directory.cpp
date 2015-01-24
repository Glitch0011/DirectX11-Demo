#include <Directory.h>

using namespace SmoothHardware;

#include <windows.h>
#include <string>
#include <vector>
#include <stack>
#include <iostream>

void DirectoryIteractor::LoadFiles()
{
	files.clear();
	
	wstring _path = this->path;

	stack<wstring> directories;
	directories.push(_path);

	while (!directories.empty())
	{
		_path = directories.top();
		wstring spec = _path + L"\\" + L"*.*";
		directories.pop();

		WIN32_FIND_DATA ffd;

		HANDLE hFind = FindFirstFile(spec.c_str(), &ffd);

		if (hFind == INVALID_HANDLE_VALUE)
			return;
		
		do
		{
			if (wcscmp(ffd.cFileName, L".") != 0 && wcscmp(ffd.cFileName, L"..") != 0) 
			{
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					directories.push(_path + L"\\" + ffd.cFileName);
				}
				else 
				{
					files.push_back(new Path(_path + L"\\" + ffd.cFileName));
				}
			}
		} 
		while (FindNextFile(hFind, &ffd) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			FindClose(hFind);
			return;
		}

		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}
}