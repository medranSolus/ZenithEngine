#include "ShadowMapCubePass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	inline Data::CBuffer::DCBLayout ShadowMapCubePass::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Array, "viewProjection");
			layout["viewProjection"].InitArray(DCBElementType::Matrix, 6);
			layout.Add(DCBElementType::Float3, "cameraPos");
			initNeeded = false;
		}
		return layout;
	}

	ShadowMapCubePass::ShadowMapCubePass(Graphics& gfx, const std::string& name, UINT mapSize)
		: QueuePass(name)
	{
		AddBindableSink<GFX::Resource::IBindable>("shadowBias");

		depthCube = GFX::Resource::TextureDepthCube::Get(gfx, mapSize, 7U);
		renderTarget = depthCube->GetBuffer();
		depthStencil = depthCube->GetStencil();
		RegisterSource(Base::SourceDirectBindable<GFX::Resource::TextureDepthCube>::Make("shadowMap", depthCube));

		positionBuffer = GFX::Resource::ConstBufferPixel<DirectX::XMFLOAT4>::Get(gfx, "$shadowMapPass");
		viewBuffer = GFX::Resource::ConstBufferExGeometryCache::Get(gfx, typeid(ShadowMapCubePass).name(), MakeLayout(), 0U);

		AddBind(positionBuffer);
		AddBind(viewBuffer);
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK, false));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));

		DirectX::XMStoreFloat4x4(&projection, DirectX::XMMatrixPerspectiveFovLH(M_PI_2, 1.0f, 0.01f, 1000.0f));
	}

	void ShadowMapCubePass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		assert(shadowSource);
		depthCube->Unbind(gfx);

		const auto& pos = shadowSource->GetPos();
		const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
		viewBuffer->GetBuffer()["cameraPos"] = mainCamera->GetPos();

		const DirectX::XMMATRIX projectionMatrix = DirectX::XMLoadFloat4x4(&projection);
		const DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		// +x
		DirectX::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][0],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)), up) * projectionMatrix));
		// -x
		DirectX::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][1],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f)), up) * projectionMatrix));
		// +y
		DirectX::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][2],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
				DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)) * projectionMatrix));
		// -y
		DirectX::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][3],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f)),
				DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)) * projectionMatrix));
		// +z
		DirectX::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][4],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)), up) * projectionMatrix));
		// -z
		DirectX::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][5],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)), up) * projectionMatrix));

		renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}