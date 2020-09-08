#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	ShadowMapPass::ShadowMapPass(Graphics& gfx, const std::string& name) : QueuePass(name)
	{
		depthCube = GFX::Resource::TextureDepthCube::Get(gfx, DEPTH_TEXTURE_SIZE, 3U);
		RegisterSource(Base::SourceDirectBindable<GFX::Resource::TextureDepthCube>::Make("depthMap", depthCube));
		depthStencil = depthCube->GetBuffer(0);

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

	void ShadowMapPass::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		assert(shadowCamera);
		gfx.SetProjection(DirectX::XMLoadFloat4x4(&projection));
		depthCube->Unbind(gfx);

		const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&shadowCamera->GetPos());
		for (unsigned char i = 0; i < 6; ++i)
		{
			depthStencil = depthCube->GetBuffer(i);
			depthStencil->Clear(gfx);
			gfx.SetCamera(DirectX::XMMatrixLookAtLH(position,
				DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&cameraDirections.at(i))), DirectX::XMLoadFloat3(&cameraUps.at(i))));
			QueuePass::Execute(gfx);
			//depthStencil->ToSurface(gfx, false).Save("__shadow" + std::to_string(i) + ".png");
		}
	}
}