#pragma once

#include <Path.h>
#include <vector>

using namespace std;

namespace SmoothHardware
{
	class DirectoryIteractor
	{
		vector<Path*> files;
		int index;
		Path path;

		void LoadFiles();

	public:
		DirectoryIteractor()
		{
			this->index = -1;
		}

		DirectoryIteractor(Path _path)
		{
			this->path = _path;
			this->LoadFiles();
			this->index = 0;
		}

		bool operator!=(DirectoryIteractor& other)
		{
			//If we are looking an an end_interactor
			if (other.getIndex() == -1)
			{
				if (this->files.size() == this->index)
					return false;
				else
					return true;
			}
			else
			{
				return other.getIndex() != this->getIndex();
			}
		}

		bool operator++()
		{
			++this->index;
			return true;
		}

		int getIndex()
		{
			return index;
		}

		Path* operator->() const
		{
			return this->files[this->index];
		}

		Path& operator*() const
		{
			return *this->files[this->index];
		}

		Path* Get()
		{
			return &this->path;
		}
	};

	class Directory
	{

	};
}