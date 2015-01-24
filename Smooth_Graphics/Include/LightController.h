#pragma once

#include <vector>
#include <string>
#include <map>

#include <LightData.h>
#include <BufferController.h>
#include <StructuredBuffer.h>
#include <MappedGpuMemory.h>

namespace SmoothGraphics
{
	class LightController
	{
		std::map<UINT, LightData*> lightData;

		StructuredBuffer* lightDataBuffer;
		BufferController* bufferController;

		UINT lightIndex = 0;
		bool dirty;

	public:
		LightController(BufferController* bufferController)
		{
			this->dirty = false;
			this->bufferController = bufferController;
		}

		HRESULT CreateLight(LightData* data);

		HRESULT UpdateLight();

		HRESULT UpdateBuffers(ID3D11DeviceContext* immediate);

		std::vector<LightData> getContiguousData()
		{
			std::vector<LightData> contiguousLightData;

			for (auto data : lightData)
			{
				contiguousLightData.push_back(*data.second);
			}

			return contiguousLightData;
		}

		UINT getCount()
		{
			return this->lightData.size();
		}
	};
}