#pragma once

#include <d3d11.h>
#include <string>

inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, const std::string& name)
{
#if defined(_DEBUG) || defined(PROFILE)
	if (resource)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
#endif
}

inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, const std::wstring& name)
{
	std::string str(name.begin(), name.end());
	SetDebugObjectName(resource, str);
}