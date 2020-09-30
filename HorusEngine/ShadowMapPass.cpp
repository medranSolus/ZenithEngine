#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	Data::CBuffer::DCBLayout ShadowMapPass::MakeLayout() noexcept
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
		depthStencil = std::make_shared<Resource::DepthStencil>(gfx, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE, Resource::DepthStencil::Usage::DepthOnly);

		positionBuffer = GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>::Get(gfx, typeid(ShadowMapPass).name(), 1U);
		biasBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(ShadowMapPass).name(), MakeLayout(), 1U);
		biasBuffer->GetBuffer()["bias"] = static_cast<float>(bias) / DEPTH_TEXTURE_SIZE;
		AddBind(GFX::Resource::ShadowRasterizer::Get(gfx, 40, 5.0f, 0.1f));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, false));

		DirectX::XMStoreFloat4x4(&projection, DirectX::XMMatrixPerspectiveFovLH(M_PI_2, 1.0f, 0.5f, 1000.0f));
		cameraDirections.reserve(6);
		cameraUps.reserve(6);
		// +x
		cameraDirections.emplace_back(1.0f, 0.0f, 0.0f);
		cameraUps.emplace_back(0.0f, 1.0f, 0.0f);
		// -x
		cameraDirections.emplace_back(-1.0f, 0.0f, 0.0f);
		cameraUps.emplace_back(0.0f, 1.0f, 0.0f);
		// +y
		cameraDirections.emplace_back(0.0f, 1.0f, 0.0f);
		cameraUps.emplace_back(0.0f, 0.0f, -1.0f);
		// -y
		cameraDirections.emplace_back(0.0f, -1.0f, 0.0f);
		cameraUps.emplace_back(0.0f, 0.0f, 1.0f);
		// +z
		cameraDirections.emplace_back(0.0f, 0.0f, 1.0f);
		cameraUps.emplace_back(0.0f, 1.0f, 0.0f);
		// -z
		cameraDirections.emplace_back(0.0f, 0.0f, -1.0f);
		cameraUps.emplace_back(0.0f, 1.0f, 0.0f);
	}

	void ShadowMapPass::Execute(Graphics& gfx)
	{
		assert(shadowSource);
		gfx.SetProjection(DirectX::XMLoadFloat4x4(&projection));
		depthCube->Unbind(gfx);

		const auto& pos = shadowSource->GetPos();
		const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 0.0f));
		positionBuffer->Bind(gfx);
		biasBuffer->Bind(gfx);
		for (unsigned char i = 0; i < 6; ++i)
		{
			renderTarget = depthCube->GetBuffer(i);
			renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
			depthStencil->Clear(gfx);
			gfx.SetView(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&cameraDirections.at(i))), DirectX::XMLoadFloat3(&cameraUps.at(i))));
			QueuePass::Execute(gfx);
		}
	}

	void ShadowMapPass::ShowWindow(Graphics& gfx)
	{
		ImGui::Text("Shadows");
		int bias = static_cast<int>(static_cast<float>(biasBuffer->GetBufferConst()["bias"]));
		if (ImGui::DragInt("Depth bias", &bias))
			biasBuffer->GetBuffer()["bias"] = static_cast<float>(bias) / DEPTH_TEXTURE_SIZE;
	}
}