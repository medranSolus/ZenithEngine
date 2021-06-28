#include "GFX/Pipeline/RenderPass/ShadowMapCubePass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	Data::CBuffer::DCBLayout ShadowMapCubePass::MakeLayout() noexcept
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

	ShadowMapCubePass::ShadowMapCubePass(Graphics& gfx, std::string&& name, U32 mapSize)
		: RenderPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name))
	{
		AddBindableSink<GFX::Resource::IBindable>("shadowBias");

		depthCube = GFX::Resource::TextureDepthCube::Get(gfx, mapSize, 0);
		renderTarget = depthCube->GetBuffer();
		depthStencil = depthCube->GetStencil();
		RegisterSource(Base::SourceDirectBindable<GFX::Resource::TextureDepthCube>::Make("shadowMap", depthCube));

		positionBuffer = GFX::Resource::ConstBufferPixel<Float4>::Get(gfx, "$shadowMapPass");
		viewBuffer = GFX::Resource::ConstBufferExGeometryCache::Get(gfx, "$SQ", MakeLayout(), 0);

		AddBind(positionBuffer);
		AddBind(viewBuffer);
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK, false));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));

		Math::XMStoreFloat4x4(&projection, Math::XMMatrixPerspectiveFovLH(static_cast<float>(M_PI_2), 1.0f, 0.01f, 1000.0f));
	}

	void ShadowMapCubePass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		assert(shadowSource);
		depthCube->Unbind(gfx);

		const auto& pos = shadowSource->GetPos();
		const Vector position = Math::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
		viewBuffer->GetBuffer()["cameraPos"] = mainCamera->GetPos();

		const Matrix projectionMatrix = Math::XMLoadFloat4x4(&projection);
		const Vector up = { 0.0f, 1.0f, 0.0f, 0.0f };
		// +x
		Math::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][0],
			Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 1.0f, 0.0f, 0.0f, 0.0f }), up) * projectionMatrix));
		// -x
		Math::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][1],
			Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { -1.0f, 0.0f, 0.0f, 0.0f }), up) * projectionMatrix));
		// +y
		Math::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][2],
			Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, 1.0f, 0.0f, 0.0f }),
				{ 0.0f, 0.0f, -1.0f, 0.0f }) * projectionMatrix));
		// -y
		Math::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][3],
			Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, -1.0f, 0.0f, 0.0f }),
				{ 0.0f, 0.0f, 1.0f, 0.0f }) * projectionMatrix));
		// +z
		Math::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][4],
			Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, 0.0f, 1.0f, 0.0f }), up) * projectionMatrix));
		// -z
		Math::XMStoreFloat4x4(&viewBuffer->GetBuffer()["viewProjection"][5],
			Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, 0.0f, -1.0f, 0.0f }), up) * projectionMatrix));

		renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}