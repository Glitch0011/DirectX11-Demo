#include <ModelController.h>

using namespace SmoothGraphics;
using namespace std;
using namespace DirectX;

#include <Directory.h>
#include <tiny_obj_loader.h>
#include <future>

ModelController::ModelController()
{
	this->GenerateSphere();
}

HRESULT ModelController::Init()
{
	std::wstring path = L".\\Assets";
	SmoothHardware::DirectoryIteractor end_itr;

	for (SmoothHardware::DirectoryIteractor file(path); file != end_itr; ++file)
	{
		if (file->extension() == L"obj")
		{
			std::shared_future<void> asyncFunction = async(launch::async, [=](wstring name, SmoothHardware::Path directory)
			{
				string str(name.begin(), name.end());

				vector<tinyobj::shape_t> shapes;
				vector<tinyobj::material_t> materials;

				auto s = (std::wstring)(directory);
				string path(s.begin(), s.end());
				path += '\\';

				string err = tinyobj::LoadObj(shapes, materials, str.c_str(), path.c_str());
				if (!err.empty())
				{
					exit(1);
				}

				auto model = new Model(file->file_name());

				for (auto itr = shapes.begin(); itr != shapes.end(); ++itr)
				{
					auto shape = new Shape(wstring(itr->name.begin(), itr->name.end()));

					for (auto i = itr->mesh.positions.begin(); i != itr->mesh.positions.end(); 0)
						shape->positions.push_back(XMFLOAT3((*i++), (*i++), (*i++)));
					
					for (auto i = itr->mesh.normals.begin(); i != itr->mesh.normals.end(); 0)
						shape->normals.push_back(XMFLOAT3((*i++), (*i++), (*i++)));

					for (auto i = itr->mesh.texcoords.begin(); i != itr->mesh.texcoords.end(); 0)
						shape->texCoords.push_back(XMFLOAT2((*i++), (*i++)));

					for (auto i = itr->mesh.material_ids.begin(); i != itr->mesh.material_ids.end(); 0)
						shape->textures.push_back(*i++);

					shape->indices = itr->mesh.indices;

					model->AddShapes(shape);
				}

				this->models.push_back(model);
			}, *file, *file.Get());
		}
	}

	return S_OK;
}

void ModelController::GenerateSphere()
{
	auto radius = 1.0f;
	auto numberOfRings = 80.0f;
	auto numberOfSlicesPerRing = 80.0f;

	vector<ModelVertex> verticies = calculateVerticies(radius, numberOfRings, numberOfSlicesPerRing);
	vector<UINT> indicies = calculateInidies(numberOfRings, numberOfSlicesPerRing + 1, numberOfSlicesPerRing + 1, verticies.size());

	auto model = new Model(L"sphere");
	auto shape = new Shape(L"sphere");

	for (auto i : verticies) shape->positions.push_back(i.Position);
	for (auto i : verticies) shape->normals.push_back(i.Normal);
	for (auto i : verticies) shape->texCoords.push_back(i.TexCord);

	shape->indices = indicies;

	model->AddShapes(shape);
	this->models.push_back(model);
}

vector<ModelVertex> ModelController::calculateVerticies(float radius, float numberOfRings, float numberOfSlicesPerRing)
{
	vector<ModelVertex> verticies;

	auto perRingAngle = XM_PI / numberOfRings;

	for (auto ringIndex = 1; ringIndex < numberOfRings; ringIndex++)
	{
		auto ringAngle = ringIndex * perRingAngle;

		auto perSliceAngle = XM_2PI / numberOfSlicesPerRing;
		for (auto sliceIndex = 0; sliceIndex < numberOfSlicesPerRing + 1; sliceIndex++)
		{
			auto sliceAngle = sliceIndex * perSliceAngle;

			auto x = radius * sin(ringAngle) * sin(sliceAngle);
			auto y = radius * cos(ringAngle);
			auto z = radius * sin(ringAngle) * cos(sliceAngle);

			auto u = sliceAngle / XM_2PI;
			auto v = ringAngle / XM_2PI;

			auto vertexTangent = XMFLOAT3(cos(sliceAngle), 0, -sin(sliceAngle));
			auto vertexNormal = XMFLOAT3(sin(ringAngle) * sin(sliceAngle), cos(ringAngle), sin(ringAngle) * cos(sliceAngle));

			ModelVertex vertex;
			vertex.Position = XMFLOAT3(x, y, z);
			vertex.Normal = vertexNormal;
			vertex.TexCord = XMFLOAT2(u, v);

			verticies.push_back(vertex);
		}
	}

	{
		ModelVertex vertex;
		vertex.Position = XMFLOAT3(0, radius, 0);
		vertex.Normal = XMFLOAT3(0, 1, 0);
		vertex.TexCord = XMFLOAT2(0, 0);

		verticies.push_back(vertex);
	}
	{
		ModelVertex vertex;
		vertex.Position = XMFLOAT3(0, -radius, 0);
		vertex.Normal = XMFLOAT3(0, -1, 0);
		vertex.TexCord = XMFLOAT2(0, 1);

		verticies.push_back(vertex);
	}

	return verticies;
}

vector<UINT> ModelController::calculateInidies(float numberOfRings, float numberOfSlicesPerRing, int numOfRingVerticies, int numOfVerticies)
{
	vector<UINT> indicies;

	for (auto ringIndex = 0; ringIndex < numberOfRings - 2; ringIndex++)
	{
		for (auto sliceIndex = 0; sliceIndex < numberOfSlicesPerRing; sliceIndex++)
		{
			indicies.push_back(sliceIndex + (numOfRingVerticies * ringIndex));
			indicies.push_back(sliceIndex + (numOfRingVerticies * (ringIndex + 1)));
			indicies.push_back((sliceIndex + 1 != numOfRingVerticies ? sliceIndex + 1 : 0) + (numOfRingVerticies * (ringIndex + 1)));

			indicies.push_back(sliceIndex + (numOfRingVerticies * ringIndex));
			indicies.push_back((sliceIndex + 1 != numOfRingVerticies ? sliceIndex + 1 : 0) + (numOfRingVerticies * (ringIndex + 1)));
			indicies.push_back((sliceIndex + 1 != numOfRingVerticies ? sliceIndex + 1 : 0) + (numOfRingVerticies * ringIndex));
		}
	}

	auto northPoleVertex = numOfVerticies - 2;
	auto southPoleVertex = numOfVerticies - 1;
	auto lastRingVertex = numOfRingVerticies * (numberOfRings - 2);

	for (auto sliceIndex = 0; sliceIndex < numberOfSlicesPerRing; sliceIndex++)
	{
		indicies.push_back(northPoleVertex);
		indicies.push_back(sliceIndex + 0);
		indicies.push_back(sliceIndex + 1 != numOfRingVerticies ? sliceIndex + 1 : 0);

		indicies.push_back(southPoleVertex);
		indicies.push_back(lastRingVertex + (sliceIndex + 0));
		indicies.push_back(lastRingVertex + (sliceIndex + 1 != numOfRingVerticies ? sliceIndex + 1 : 0));
	}

	return indicies;
}