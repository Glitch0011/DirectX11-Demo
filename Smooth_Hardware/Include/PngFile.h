#pragma once

#include <File.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <map>
#include <winerror.h>
#include <istream>

#include <Windows.h>

#include <vector>
#include <picopng.h>

using namespace std;

#define SQUARE(x) pow(x, 2.0)

namespace SmoothHardware
{
	typedef unsigned char UINT8;

	class PngFile : public File
	{
	public:
		FLOAT* data;
		UINT width, height;

		PngFile(Path path) :
			File(path)
		{
			LoadPNG(path);
		}

		~PngFile()
		{
			delete[] this->data;
		}

		HRESULT LoadPNG(const wstring location)
		{
			std::vector<unsigned char> buffer, image;

			loadFile(buffer, std::string(location.begin(), location.end()));

			unsigned long w, h;
			int error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

			this->width = w;
			this->height = h;

			this->data = new FLOAT[this->width * this->height * 4];

			for (UINT i = 0; i < image.size(); i++)
			{
				this->data[i] = (float)image[i]/255.0f;
			}

			return S_OK;
		}
	};
}