#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	inline Data::CBuffer::DCBLayout ShadowMapPass::MakeLayoutView() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Array, "viewProjection");
			layout["viewProjection"].InitArray(DCBElementType::Matrix, 6);
			initNeeded = false;
		}
		return layout;
	}

	inline Data::CBuffer::DCBLayout ShadowMapPass::MakeLayoutBias() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Float, "bias");
			initNeeded = false;
		}
		return layout;
	}

	ShadowMapPass::ShadowMapPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name)
	{
		depthCube = GFX::Resource::TextureDepthCube::Get(gfx, DEPTH_TEXTURE_SIZE, 7U);
		RegisterSource(Base::SourceDirectBindable<GFX::Resource::TextureDepthCube>::Make("depthMap", depthCube));

		positionBuffer = GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>::Get(gfx, typeid(ShadowMapPass).name(), 1U);
		viewBuffer = GFX::Resource::ConstBufferExGeometryCache::Get(gfx, typeid(ShadowMapPass).name(), MakeLayoutView(), 0U);
		biasBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(ShadowMapPass).name(), MakeLayoutBias(), 1U);
		biasBuffer->GetBuffer()["bias"] = static_cast<float>(bias) / DEPTH_TEXTURE_SIZE;

		AddBind(positionBuffer);
		AddBind(viewBuffer);
		AddBind(biasBuffer);
		AddBind(GFX::Resource::ShadowRasterizer::Get(gfx, 40, 5.0f, 0.1f));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, false));

		DirectX::XMStoreFloat4x4(&projection, DirectX::XMMatrixPerspectiveFovLH(M_PI_2, 1.0f, 0.5f, 1000.0f));
	}

	void ShadowMapPass::Execute(Graphics& gfx)
	{
		assert(shadowSource);
		depthCube->Unbind(gfx);

		const auto& pos = shadowSource->GetPos();
		const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });

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

		renderTarget = depthCube->GetBuffer();
		depthStencil = depthCube->GetStencil();
		renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}

	void ShadowMapPass::ShowWindow(Graphics& gfx)
	{
		if (ImGui::DragInt("Shadow depth bias", &bias))
			biasBuffer->GetBuffer()["bias"] = static_cast<float>(bias) / DEPTH_TEXTURE_SIZE;
	}
}