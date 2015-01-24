#pragma once

#include <File.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <map>
#include <winerror.h>
#include <istream>

#include <Windows.h>

using namespace std;

#define SQUARE(x) pow(x, 2.0)

namespace SmoothHardware
{
	typedef unsigned char UINT8;

	class BmpFile : public File
	{
	public:
		FLOAT* data;
		UINT width, height;

		BmpFile(Path path) :
			File(path)
		{
			LoadBMP(path);
		}

		~BmpFile()
		{
			delete[] this->data;
		}

		HRESULT LoadBMP(const wstring location)
		{
			UINT8* datBuff[2] = { nullptr, nullptr }; // Header buffers

			BITMAPFILEHEADER* bmpHeader = nullptr; // Header
			BITMAPINFOHEADER* bmpInfo = nullptr; // Info 

			std::ifstream file(location, std::ios::binary);
			if (!file)
			{
				std::cout << "Failure to open bitmap file.\n";
				return E_INVALIDARG;
			}

			datBuff[0] = new UINT8[sizeof(BITMAPFILEHEADER)];
			datBuff[1] = new UINT8[sizeof(BITMAPINFOHEADER)];

			file.read((char*)datBuff[0], sizeof(BITMAPFILEHEADER));
			file.read((char*)datBuff[1], sizeof(BITMAPINFOHEADER));

			// Construct the values from the buffers
			bmpHeader = (BITMAPFILEHEADER*)datBuff[0];
			bmpInfo = (BITMAPINFOHEADER*)datBuff[1];

			// Check if the file is an actual BMP file
			if (bmpHeader->bfType != 0x4D42)
			{
				return E_INVALIDARG;
			}
			
			this->width = bmpInfo->biWidth;
			this->height = bmpInfo->biHeight;

			// First allocate pixel memory
			UINT8* tempData = new UINT8[bmpInfo->biSizeImage];
			ZeroMemory(tempData, sizeof(UINT8) * bmpInfo->biSizeImage);

			// Go to where image data starts, then read in image data
			file.seekg(bmpHeader->bfOffBits);
			file.read((char*)tempData, bmpInfo->biSizeImage);

			this->data = new FLOAT[this->width * this->height * 4];

			// We're almost done. We have our image loaded, however it's not in the right format.
			// .bmp files store image data in the BGR format, and we have to convert it to RGB.
			// Since we have the value in bytes, this shouldn't be to hard to accomplish
			for (ULONG i = 0, j = 0; i < this->width * this->height * 3; i += 3, j += 4)
			{
				this->data[j] = tempData[i + 2] / 255.0;
				this->data[j + 1] = tempData[i + 1] / 255.0;
				this->data[j + 2] = tempData[i] / 255.0;
				this->data[j + 3] = 255 / 255.0;

				/*double length = sqrt(SQUARE(this->data[j]) + SQUARE(this->data[j + 1]) + SQUARE(this->data[j + 2]) + SQUARE(this->data[j + 3]));
  				auto itr = &this->data[j];
				*itr = (*itr / length); itr++;
				*itr = (*itr / length); itr++;
				*itr = (*itr / length); itr++;
				*itr = (*itr / length); itr++;*/
			}

			delete[] tempData;

			file.close();
		}
	};
}