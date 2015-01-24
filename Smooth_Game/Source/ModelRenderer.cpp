#include <ModelRenderer.h>

#include <Component.h>
#include <string>
#include <Model.h>
#include <VertexBuffer.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <Renderer.h>
#include <vector>
#include <GameObject.h>
#include <IndexBuffer.h>

#include <ConstantBuffer.h>
#include <PositionData.h>
#include <Collider.h>

using namespace DirectX;
using namespace SmoothGraphics;
using namespace SmoothGame;
using namespace std;

bool ModelRenderer::ShouldDraw()
{
	return !this->GameObject->SendAndRecieve<bool>(L"isOutsideFrustum", { &(*this->frustum) });
}

ModelRenderer::ModelRenderer(std::wstring modelName, std::wstring textureName, float ambient) : Renderer()
{
	this->highlightData.vertexid = -1;
	this->highlightData.id = -1;
	this->highlightData.c = -1;
	this->highlightData.d = -1;

	this->ambient = XMFLOAT4(ambient, ambient, ambient, ambient);

	this->modelName = modelName;
	this->textureName = textureName;
	this->pixelShaderName = L"ModelPixelShader";

	samplerState = CachedVariable<ID3D11SamplerState>(TO_FUNCTION(this->graphics->samplerStates[L"samplerState"]));
	vertexShader = CachedVariable<VertexShader>(TO_FUNCTION(this->graphics->GetShader<VertexShader>(L"ModelVertexShader")));
	geometryShader = CachedVariable<GeometryShader>(TO_FUNCTION(this->graphics->GetShader<GeometryShader>(this->geometryName)));

	pixelShader = CachedVariable<PixelShader>(TO_FUNCTION(this->graphics->GetShader<PixelShader>(this->pixelShaderName)));
	texture = CachedVariable<TextureBuffer>(TO_FUNCTION(this->graphics->GetResource<TextureBuffer>(this->textureName)));
	environmentTexture = CachedVariable<TextureBuffer>(TO_FUNCTION(this->graphics->GetResource<TextureBuffer>(this->environmentName)));
	bumpTexture = CachedVariable<TextureBuffer>(TO_FUNCTION(this->graphics->GetResource<TextureBuffer>(this->bumpName)));
	heightMapTexture = CachedVariable<TextureBuffer>(TO_FUNCTION(this->graphics->GetResource<TextureBuffer>(this->heightMapName)));

	quickConstantBuffer = CachedVariable<Buffer>(TO_FUNCTION(this->graphics->Get(L"QuickBasicConstantBuffer")));
	basicConstantBuffer = CachedVariable<Buffer>(TO_FUNCTION(this->graphics->Get(L"BasicConstantBuffer")));
	ambientConstantBuffer = CachedVariable<Buffer>(TO_FUNCTION(this->graphics->Get(L"AmbientConstantBuffer")));
	textureAvaliableConstantBuffer = CachedVariable<Buffer>(TO_FUNCTION(this->graphics->Get(L"TextureAvaliableConstantBuffer")));

	frustum = CachedVariable<Frustum>(TO_FUNCTION(this->GameObject->SendAndRecieve<Frustum*>(L"getFrustum")));

	highlightingDataConstantBuffer = CachedVariable<Buffer>(TO_FUNCTION(this->graphics->Get(L"highlightingData")));

	this->functions[L"Render"] = [=](Params param)
	{
		if (this->model && this->objData)
		{
			if (this->ShouldDraw())
			{
				for (auto shape : this->model->shapes)
				{
					auto vertexBuffer = this->graphics->Get<VertexBuffer>(shape->name + L"_vertex");
					auto indexBuffer = this->graphics->Get<IndexBuffer>(shape->name + L"_index");

					auto context = this->graphics->immediateContext;

					if (vertexBuffer && indexBuffer)
					{
						if (vertexShader && pixelShader)
						{
							IF_NOT_NULL(vertexShader, vertexShader->SetInputLayout(context));
							IF_NOT_NULL(vertexShader, vertexShader->Set(context));
							IF_NOT_NULL(pixelShader, pixelShader->Set(context));
							IF_NOT_NULL(geometryShader, geometryShader->Set(context));

							//Set texture buffers
							IF_NOT_NULL(samplerState, context->PSSetSamplers(0, 1, &samplerState.value));
							IF_NOT_NULL(texture, context->PSSetShaderResources(0, 1, &texture->srvResourceView));
							IF_NOT_NULL(environmentTexture, context->PSSetShaderResources(1, 1, &environmentTexture->srvResourceView));
							IF_NOT_NULL(bumpTexture, context->PSSetShaderResources(2, 1, &bumpTexture->srvResourceView));
							IF_NOT_NULL(heightMapTexture, context->PSSetShaderResources(3, 1, &heightMapTexture->srvResourceView));

							vertexBuffer->Set(context);
							indexBuffer->Set(context);

							//Set vertex constants
							IF_NOT_NULL(basicConstantBuffer, context->VSSetConstantBuffers(0, 1, &basicConstantBuffer->buffer));
							IF_NOT_NULL(quickConstantBuffer, context->VSSetConstantBuffers(1, 1, &quickConstantBuffer->buffer));

							//Set pixel constants
							IF_NOT_NULL(basicConstantBuffer, context->PSSetConstantBuffers(0, 1, &basicConstantBuffer->buffer));
							IF_NOT_NULL(ambientConstantBuffer, context->PSSetConstantBuffers(1, 1, &ambientConstantBuffer->buffer));
							IF_NOT_NULL(textureAvaliableConstantBuffer, context->PSSetConstantBuffers(2, 1, &textureAvaliableConstantBuffer->buffer));
							IF_NOT_NULL(highlightingDataConstantBuffer, context->PSSetConstantBuffers(3, 1, &highlightingDataConstantBuffer->buffer));

							//Set geometry constants
							IF_NOT_NULL(basicConstantBuffer, context->GSSetConstantBuffers(0, 1, &basicConstantBuffer->buffer));

							//Build world matrix
							auto worldMatrix =
								XMMatrixRotationX(objData->Rotation()->x) *
								XMMatrixRotationY(objData->Rotation()->y) *
								XMMatrixRotationZ(objData->Rotation()->z) *
								XMMatrixScalingFromVector(XMLoadFloat3(objData->Scale())) *
								XMMatrixTranslationFromVector(XMLoadFloat3(objData->Position())) *
								XMMatrixIdentity(); //Just to allow chaining

							//Map the matrix data across
							QuickBasicConstantBuffer _quickConstantBuffer;
							_quickConstantBuffer.mWorld = XMMatrixTranspose(worldMatrix);
							{
								MappedGpuMemory mappedMemory(context, quickConstantBuffer->Get());

								HRESULT hr;
								if (FAILED(hr = mappedMemory.Set<QuickBasicConstantBuffer>(&_quickConstantBuffer)))
									return hr;
							}

							//Map the ambient data across;
							AmbientConstantBuffer _ambentConstantBuffer;
							_ambentConstantBuffer.ambient = XMLoadFloat4(&this->ambient);
							_ambentConstantBuffer.modelID = this->GameObject->ID;
							_ambentConstantBuffer.highlight = this->GameObject->ID == this->highlightData.id ? true : false;
							{
								MappedGpuMemory mappedMemory(context, ambientConstantBuffer->Get());

								HRESULT hr;
								if (FAILED(hr = mappedMemory.Set<AmbientConstantBuffer>(&_ambentConstantBuffer)))
									return hr;
							}

							//Map the texture avaliability across
							TextureAvaliabilityConstantBuffer _textureAvaliableConstantBuffer;
							_textureAvaliableConstantBuffer.textureMapped = texture ? 1 : 0;
							_textureAvaliableConstantBuffer.bumpMapped = bumpTexture ? 1 : 0;
							_textureAvaliableConstantBuffer.heightMapped = heightMapTexture ? 1 : 0;
							_textureAvaliableConstantBuffer.environmentMapped = environmentTexture ? 1 : 0;
							{
								MappedGpuMemory mappedMemory(context, textureAvaliableConstantBuffer->Get());

								HRESULT hr;
								if (FAILED(hr = mappedMemory.Set<TextureAvaliabilityConstantBuffer>(&_textureAvaliableConstantBuffer)))
									return hr;
							}

							//Map the picking data across
							{
								MappedGpuMemory mappedMemory(context, highlightingDataConstantBuffer->Get());

								HRESULT hr;
								if (FAILED(hr = mappedMemory.Set<PickingData>(&this->highlightData)))
									return hr;
							}

							this->graphics->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

							IF_NOT_NULL(indexBuffer, indexBuffer->Draw(context));

							this->graphics->UnsetVertexBuffer();

							this->graphics->immediateContext->GSSetShader(nullptr, nullptr, 0);

							ID3D11ShaderResourceView* blankView = nullptr;
							context->PSSetShaderResources(0, 1, &blankView);
							context->PSSetShaderResources(1, 1, &blankView);
							context->PSSetShaderResources(2, 1, &blankView);
							context->PSSetShaderResources(3, 1, &blankView);
						}
					}
					else if (!vertexBuffer && !indexBuffer)
					{
						SetupVertexBuffers(shape);
					}
				}
			}
		}
		else if (model == nullptr)
		{
			this->GameObject->SendAndRecieveAsync<Model*>(L"getModel", { &this->modelName }, [=](Model* model)
			{
				this->model = model;
			});

			return S_OK;
		}

		return S_OK;
	};

	this->functions[L"getModelRenderer"] = [=](Params param)
	{
		auto wrapper = (SendAndRecieveWrapper*)param[0];

		auto w = *((std::wstring**)wrapper->params[0]);
		if (*w == this->modelName)
			wrapper->result = this;

		return S_OK;
	};
}

bool ModelRenderer::isRunning()
{
	if (!this->model)
		return false;

	if (this->model && this->objData)
	{
		for (auto shape : this->model->shapes)
		{
			auto vertexBuffer = this->graphics->Get<VertexBuffer>(shape->name + L"_vertex");
			auto indexBuffer = this->graphics->Get<IndexBuffer>(shape->name + L"_index");
			auto vertexShader = this->graphics->GetShader<VertexShader>(L"ModelVertexShader");
			auto pixelShader = this->graphics->GetShader<PixelShader>(L"ModelPixelShader");

			if (!vertexBuffer || !indexBuffer || !vertexShader || !pixelShader)
			{
				return false;
			}
		}
	}
	return true;
}

void ModelRenderer::SetupVertexBuffers(Shape* shape)
{
	this->graphics->CreateVertexBuffer<ModelVertex>(shape->name + L"_vertex", shape->positions.size(), [=](UINT index, ModelVertex* vertex)
	{
		vertex->Position = shape->positions[index];
		vertex->Normal = index < shape->normals.size() ? shape->normals[index] : XMFLOAT3();
		vertex->TexCord = index < shape->texCoords.size() ? shape->texCoords[index] : XMFLOAT2();
		return S_OK;
	});
	this->graphics->CreateIndexBuffer<unsigned int>(shape->name + L"_index", shape->indices.size(), [=](UINT index, unsigned int* indicie)
	{
		*indicie = shape->indices[index];
		return S_OK;
	});
}

HRESULT ModelRenderer::Init()
{
	this->GameObject->SendAndRecieveAsync<PositionalData*>(L"getPositionData", [=](PositionalData* data)
	{
		this->objData = data;
	});

	return Renderer::Init();
}