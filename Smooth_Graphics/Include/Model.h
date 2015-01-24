#pragma once

#include <vector>
#include <NamedResource.h>
#include <DirectXMath.h>
#include <d3d11.h>

namespace SmoothGraphics
{
	class Shape : public NamedResource
	{
	public:
		std::vector<DirectX::XMFLOAT3> positions;
		std::vector<UINT> indices;
		std::vector<DirectX::XMFLOAT3> normals;
		std::vector<DirectX::XMFLOAT2> texCoords;
		std::vector<UINT> textures;

		Shape(std::wstring name) :
			NamedResource(name)
		{

		}
	};

	class Model : public NamedResource
	{
	public:
		std::vector<Shape*> shapes;

		Model(std::wstring name) :
			NamedResource(name)
		{

		}

		void AddShapes(Shape* shape)
		{
			this->shapes.push_back(shape);
		}
	};
}