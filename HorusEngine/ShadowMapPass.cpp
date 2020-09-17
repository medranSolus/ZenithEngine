#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	ShadowMapPass::ShadowMapPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name)
	{
		depthCube = GFX::Resource::TextureDepthCube::Get(gfx, DEPTH_TEXTURE_SIZE, 8U);
		RegisterSource(Base::SourceDirectBindable<GFX::Resource::TextureDepthCube>::Make("depthMap", depthCube));
		depthStencil = depthCube->GetBuffer(0);

		positionBuffer = GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>::Get(gfx, typeid(ShadowMapPass).name(), 1U);
		AddBind(GFX::Resource::NullPixelShader::Get(gfx));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, false));
		AddBind(GFX::Resource::ShadowRasterizer::Get(gfx, 40, 3.0f, 0.1f));

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
		for (unsigned char i = 0; i < 6; ++i)
		{
			depthStencil = depthCube->GetBuffer(i);
			depthStencil->Clear(gfx);
			gfx.SetView(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&cameraDirections.at(i))), DirectX::XMLoadFloat3(&cameraUps.at(i))));
			QueuePass::Execute(gfx);
		}
	}
}