#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	ShadowMapPass::ShadowMapPass(Graphics& gfx, const std::string& name, const DirectX::XMMATRIX& projectionMatrix)
		: QueuePass(name)
	{
		AddBindableSink<GFX::Resource::IBindable>("shadowBias");

		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("shadowMapDepth", depthStencil));
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("shadowMapTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depth", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("scratch", renderTarget));
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("shadowMap", renderTarget));

		positionBuffer = GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>::Get(gfx, typeid(ShadowMapPass).name(), 3U);

		AddBind(positionBuffer);
		AddBind(GFX::Resource::ShadowRasterizer::Get(gfx, 40, 5.0f, 0.1f));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));

		DirectX::XMStoreFloat4x4(&projection, projectionMatrix);
	}

	void ShadowMapPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		assert(shadowSource);

		const auto& pos = shadowSource->GetPos();
		const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
		mainCamera->BindVS(gfx);

		gfx.SetView(DirectX::XMMatrixLookAtLH(position,
			DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&shadowSource->GetBuffer()["direction"])),
			DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)));
		gfx.SetProjection(DirectX::XMLoadFloat4x4(&projection));

		renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}