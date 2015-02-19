#include <SpriteRenderer.h>

using namespace SmoothGame;

SpriteRenderer::SpriteRenderer(std::wstring textureName)
{
	this->textureName = textureName;

	this->samplerState = CachedVariable<ID3D11SamplerState>(TO_FUNCTION(this->graphics->samplerStates[L"samplerState"]));
	this->texture = CachedVariable<TextureBuffer>(TO_FUNCTION(this->graphics->GetResource<TextureBuffer>(textureName)));

	this->positionData = CachedVariable<PositionalData>(TO_FUNCTION(this->GameObject->SendAndRecieve<PositionalData*>(L"getPositionData")));

	this->vShader = CachedVariable<VertexShader>(TO_FUNCTION(this->graphics->GetShader<VertexShader>(L"Basic_VS")));
	this->pShader = CachedVariable<PixelShader>(TO_FUNCTION(this->graphics->GetShader<PixelShader>(L"Basic_PS")));

	this->basicConstantBuffer = CachedVariable<ConstantBuffer>(TO_FUNCTION(this->graphics->Get<ConstantBuffer>(L"BasicConstantBuffer")));
	this->quickConstantBuffer = CachedVariable<DynamicConstantBuffer>(TO_FUNCTION(this->graphics->Get<DynamicConstantBuffer>(L"QuickBasicConstantBuffer")));

	this->indexBuffer = CachedVariable<IndexBuffer>(TO_FUNCTION(this->graphics->Get<IndexBuffer>(L"BasicIndexBuffer")));
	this->vertexBuffer = CachedVariable<VertexBuffer>(TO_FUNCTION(this->graphics->Get<VertexBuffer>(L"BasicVertexBuffer")));

	this->functions[L"Render"] = [=](Params param) mutable
	{
		auto context = this->graphics->immediateContext;

		if (this->vShader == nullptr)
			return S_OK;

		this->vShader->SetInputLayout(context);

		this->vShader->Set(context);
		this->pShader->Set(context);

		context->VSSetConstantBuffers(0, 1, &this->basicConstantBuffer->buffer);
		context->VSSetConstantBuffers(1, 1, &this->quickConstantBuffer->buffer);

		{
			auto worldMatrix =
				XMMatrixScalingFromVector(XMLoadFloat3(positionData->Scale())) *
				XMMatrixTranslationFromVector(XMLoadFloat3(positionData->Position())) *
				XMMatrixRotationX(positionData->Rotation()->x) *
				XMMatrixRotationY(positionData->Rotation()->y) *
				XMMatrixRotationZ(positionData->Rotation()->z);

			QuickBasicConstantBuffer b;
			b.mWorld = worldMatrix;
			
			this->quickConstantBuffer->Update(context, &b);
		}

		this->graphics->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		this->indexBuffer->Set(context);
		this->vertexBuffer->Set(context);

		IF_NOT_NULL(this->samplerState, context->PSSetSamplers(0, 1, &this->samplerState.value));
		IF_NOT_NULL(this->texture, context->PSSetShaderResources(0, 1, &this->texture->srvResourceView));

		//auto gridSystem = this->graphics->Get<StructuredBuffer>(L"GridSystem");
		//context->PSSetShaderResources(2, 1, &gridSystem->srvResourceView);
		
		//context->PSSetConstantBuffers(0, 1, &this->graphics->Get<DynamicConstantBuffer>(L"ElementConstantBuffer")->buffer);
		//context->PSSetConstantBuffers(2, 1, &this->graphics->Get<DynamicConstantBuffer>(L"ScreenConstantBuffer")->buffer);

		indexBuffer->Draw(context);

		ID3D11ShaderResourceView* const g_pNullSRV = nullptr;
		ID3D11Buffer* const nullBuffer = nullptr;

		context->VSSetShaderResources(0, 1, &g_pNullSRV);

		context->PSSetShaderResources(0, 1, &g_pNullSRV);
		context->PSSetShaderResources(2, 1, &g_pNullSRV);
		context->PSSetConstantBuffers(0, 1, &nullBuffer);

		return S_OK;
	};
}

HRESULT SpriteRenderer::Init()
{
	auto res = Renderer::Init();

	this->graphics->CreateVertexBuffer<SimpleRenderVertex>(L"BasicVertexBuffer", 4, [=](UINT index, SimpleRenderVertex* vertex)
	{
		SimpleRenderVertex v[]
		{
			SimpleRenderVertex(-0.5, -0.5, 0, 0),
			SimpleRenderVertex(-0.5, 0.5, 0, 1),
			SimpleRenderVertex(0.5, 0.5, 1, 1),
			SimpleRenderVertex(0.5, -0.5, 1, 0),
		};
		*vertex = v[index];
		return S_OK;
	});

	this->graphics->CreateIndexBuffer<int>(L"BasicIndexBuffer", 6, [=](UINT index, int* vertex)
	{
		int v[]
		{
			0, 1, 2,
			0, 3, 2,
		};
		*vertex = v[index];
		return S_OK;
	});

	return res;
}